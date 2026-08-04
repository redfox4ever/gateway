#ifndef EXTERNAL_FIRMWARE_UPDATE_H
#define EXTERNAL_FIRMWARE_UPDATE_H
#include <stdio.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/toolchain.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/bluetooth/gatt.h>
#include <stdio.h>
#include <main.h>
extern int start_firmware_update;
void firmware_update();
extern struct bt_gatt_write_params payload_size_write_params; 
void send_payload_size(int32_t payload_size_param);
#endif