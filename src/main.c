/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include <logger.h>
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
LOG_MODULE_REGISTER(main_log, LOG_LEVEL_INF);
int main(void)
{
	init_logging();
	/*
	int err;
	err = bt_enable(NULL);
    if (err) {
        printf("Bluetooth init failed (err %d)", err);
        return err;
    }
	*/
	return 0;
}
