#include <command.h>

#include <stdint.h>
volatile uint8_t cmd_feedback = NO_FEEDBACK;
volatile uint8_t cmd = NO_CMD;
struct bt_gatt_subscribe_params cmd_feedback_subscribe_params;
void cmd_write_cb(struct bt_conn *conn, uint8_t err, struct bt_gatt_write_params *params){
	printf("a cmd write request was sent!\n");
}
struct bt_gatt_write_params cmd_write_params = {
	.func = cmd_write_cb,
	.data = &cmd,
	.offset = 0,
	.length = sizeof(cmd) 
	
};
extern uint64_t total_rx_count;
uint8_t cmd_feedback_notify_func(struct bt_conn *conn,
			   struct bt_gatt_subscribe_params *params,
			   const void *data, uint16_t length)
{
	if (!data) {
		printk("[UNSUBSCRIBED]\n");
		params->value_handle = 0U;
		return BT_GATT_ITER_STOP;
	}

	printk("[NOTIFICATION] data %p length %u\n", data, length);
    printk("this is the feedback notification :)\n");

	if (length == sizeof(cmd_feedback)) {
		uint8_t cmd_feedback_value;

		memcpy(&cmd_feedback_value, data, sizeof(cmd_feedback_value));
        cmd_feedback = cmd_feedback_value;
		printk("[NOTIFICATION] cmd feedback value: %d\n", cmd_feedback_value);
	} else {
		printk("[NOTIFICATION] unexpected length %u\n", length);
	}
	total_rx_count++;

	return BT_GATT_ITER_CONTINUE;
}