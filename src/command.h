#ifndef COMMAND_H
#define COMMAND_H
#include <stdint.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/byteorder.h>
enum Commands{
	NO_CMD = 0,
	UPDATE_CMD = 1,
};

enum CmdFeedbacks{
	NO_FEEDBACK = 0,
	OTA_UPDATE_READY = 1,
	OTA_UPDATE_NOT_READY = 2,
};
extern volatile uint8_t cmd_feedback ;
extern volatile uint8_t cmd;
extern struct bt_gatt_write_params cmd_write_params;
extern struct bt_gatt_subscribe_params cmd_feedback_subscribe_params;
void cmd_write_cb(struct bt_conn *conn, uint8_t err, struct bt_gatt_write_params *params);
uint8_t cmd_feedback_notify_func(struct bt_conn *conn,
			   struct bt_gatt_subscribe_params *params,
			   const void *data, uint16_t length);
#endif