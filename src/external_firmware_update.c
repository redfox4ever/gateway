#include <external_firmware_update.h>

LOG_MODULE_REGISTER(external_firmware_update, LOG_LEVEL_INF);


static int32_t payload_size;

static void payload_size_write_cb(struct bt_conn *conn, uint8_t err, struct bt_gatt_write_params *params){
    // add proper error handling with err
    LOG_INF("payload size has been written.\n");
}

struct bt_gatt_write_params payload_size_write_params = {
	.func = payload_size_write_cb,
	.data = &payload_size,
	.offset = 0,
	.length = sizeof(payload_size) 
	
};
void send_payload_size(int32_t payload_size_param){
    // ADD value checks
    payload_size = payload_size_param;
	bt_gatt_write(default_conn, &payload_size_write_params);
}




// sending payload chunk


#define CHUNK_DATA_SIZE 239
struct PayloadChunk
{
	int32_t index;
	char data[CHUNK_DATA_SIZE];
	uint8_t len;
} __packed;

static struct PayloadChunk payload_chunk;
K_SEM_DEFINE(payload_write_done, 0, 1);
static void payload_chunk_write_cb(struct bt_conn *conn, uint8_t err, struct bt_gatt_write_params *params){
    // add proper error handling with err
    LOG_INF("payload chunk has been written.\n");
    k_sem_give(&payload_write_done);
}

struct bt_gatt_write_params payload_chunk_write_params = {
	.func = payload_chunk_write_cb,
	.data = &payload_chunk,
	.offset = 0,
	.length = sizeof(payload_chunk) 
	
};
void send_payload_chunk(struct PayloadChunk pc){
    // ADD value checks
    // TODO: make sure that all values are reasonable
    payload_chunk = pc;
	bt_gatt_write(default_conn, &payload_chunk_write_params);
    k_sem_take(&payload_write_done, K_FOREVER);
}




enum PayloadChunkFeedback{
	NO_FEEDBACK = 0,
	CHUNK_MISSED = 1,
	CHUNK_ACCEPTED = 2,
};
static uint8_t payload_chunk_feedback = NO_FEEDBACK;

struct bt_gatt_subscribe_params payload_chunk_feedback_subscribe_params;


// setting up the payload chunk feedback semaphore chunk
K_SEM_DEFINE(payload_chunk_feedback_flag, 0, 1);

//TODO: Include total rx count?
uint8_t payload_chunk_feedback_notify_func(struct bt_conn *conn,
			   struct bt_gatt_subscribe_params *params,
			   const void *data, uint16_t length)
{
	if (!data) {
		printk("[UNSUBSCRIBED]\n");
		params->value_handle = 0U;
		return BT_GATT_ITER_STOP;
	}

	printf("[NOTIFICATION] data %p length %u\n", data, length);
    printf("this is the payload chunk feedback notification :)\n");

	if (length == sizeof(payload_chunk_feedback)) {
		uint8_t payload_chunk_feedback_value;

		memcpy(&payload_chunk_feedback_value, data, sizeof(payload_chunk_feedback_value));
        payload_chunk_feedback = payload_chunk_feedback_value;
		printk("[NOTIFICATION] payload chunk feedback value: %d\n", payload_chunk_feedback_value);
	} else {
		printk("[NOTIFICATION] unexpected length %u\n", length);
	}
	//total_rx_count++;

    k_sem_give(&payload_chunk_feedback_flag);

	return BT_GATT_ITER_CONTINUE;
}

int start_firmware_update = 1;

void firmware_update(){
    
    const struct flash_area* image_update_partition;
    int err = flash_area_open(PARTITION_ID(image_update_partition), &image_update_partition);

    if (err)
    {
        printf("failed to open image_update_partition\n");
        return ;
    }
    else
    {
        printf("image_update_partition is open.\n");
    }

    size_t image_size = PARTITION_SIZE(image_update_partition);
    send_payload_size(image_size);

    // Setup
    int32_t chunks_count = image_size / CHUNK_DATA_SIZE + ((image_size % CHUNK_DATA_SIZE ) == 0 ? 0 : 1);
    int32_t chunk_index = 0;
    bool finished_last_chunk = false;
    while(chunk_index < chunks_count){
        printf("Attempting external firmware update loop with index: %d\n", chunk_index);
        if(chunk_index == chunks_count -1) finished_last_chunk = true;
       struct PayloadChunk pc;
       pc.index = chunk_index;
       if(chunk_index == chunks_count - 1 && (image_size % CHUNK_DATA_SIZE ) != 0){
            pc.len =  image_size % CHUNK_DATA_SIZE;
       } 
       else{
            pc.len = CHUNK_DATA_SIZE;
       }
       flash_area_read(image_update_partition, chunk_index * CHUNK_DATA_SIZE, pc.data, pc.len);
       send_payload_chunk(pc);
       printf("Waiting for the semaphore to release\n");
       k_sem_take(&payload_chunk_feedback_flag, K_FOREVER);
       printf("The semaphore has release\n");
       if(payload_chunk_feedback == CHUNK_ACCEPTED){
        chunk_index++;
       } else if(payload_chunk_feedback == CHUNK_MISSED){
        // repeat the same chunk
       } else {
        printf("unknow payload chunk feedback\n");
       }
       
	   if(finished_last_chunk) break;
    }

    flash_area_close(image_update_partition);

	printf("External firmware update is done.\n");
}