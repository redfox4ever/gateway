#ifndef LOGGER_H
#define LOGGER_H

#include <sample_usbd.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/ring_buffer.h>

#include <zephyr/usb/usbd.h>
#include <zephyr/logging/log.h>
void init_logging();

#endif