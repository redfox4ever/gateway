#ifndef EXTERNAL_FIRMWARE_UPDATE_H
#define EXTERNAL_FIRMWARE_UPDATE_H
#include <stdio.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/toolchain.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
extern int start_firmware_update;
void firmware_update();
#endif