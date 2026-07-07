/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main_log, LOG_LEVEL_INF);

#include <zephyr/drivers/gpio.h>

#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>


int main(void)
{

		int err;
	err = bt_enable(NULL);
    if (err) {
        printf("Bluetooth init failed (err %d)", err);
        return err;
    }
	
	static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
	gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);

	while (1) {
		gpio_pin_toggle_dt(&led);
		k_msleep(500);
	}


		
	
	return 0;
}
