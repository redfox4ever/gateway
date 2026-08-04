#include <external_firmware_update.h>

LOG_MODULE_REGISTER(external_firmware_update, LOG_LEVEL_INF);

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

    send_payload_size(PARTITION_SIZE(image_update_partition));


}
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