/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/byteorder.h>
// LOG_MODULE_REGISTER(main_log, LOG_LEVEL_INF);
static void start_scan(void);


#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)


/* Target service :*/
#define BT_UUID_CUSTOM_SERVICE_VAL \
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

static const struct bt_uuid_128 vnd_uuid = BT_UUID_INIT_128(
	BT_UUID_CUSTOM_SERVICE_VAL);

// sensor notification -------------------------------------------------------->
static const struct bt_uuid_128 vnd_sensor_uuid = BT_UUID_INIT_128(
BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef5));




// message -------------------------------------------------------->
// message characteristic 
static const struct bt_uuid_128 vnd_msg_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef6));
#define MSG_MAX_LEN 32
char msg[MSG_MAX_LEN + 1] = {0};
uint16_t msg_attr_handle;
void msg_write_cb(struct bt_conn *conn, uint8_t err, struct bt_gatt_write_params *params){
	printf("a msg write request was sent!\n");
}
struct bt_gatt_write_params msg_write_params = {
	.func = msg_write_cb,
	.data = msg,
	.offset = 0,
	.length = MSG_MAX_LEN 
	
};
bool msg_ready = false;
// message -------------------------------------------------------->




static struct bt_conn *default_conn;

uint64_t total_rx_count; /* This value is exposed to test code */
static struct bt_uuid_128 discover_uuid = BT_UUID_INIT_128(0);
static struct bt_gatt_discover_params discover_params;
static struct bt_gatt_subscribe_params subscribe_params;


static uint8_t notify_func(struct bt_conn *conn,
			   struct bt_gatt_subscribe_params *params,
			   const void *data, uint16_t length)
{
	if (!data) {
		printk("[UNSUBSCRIBED]\n");
		params->value_handle = 0U;
		return BT_GATT_ITER_STOP;
	}

	printk("[NOTIFICATION] data %p length %u\n", data, length);

	if (length == sizeof(int16_t)) {
		int16_t sensor_value;

		memcpy(&sensor_value, data, sizeof(sensor_value));
		sensor_value = sys_le16_to_cpu(sensor_value);

		printk("[NOTIFICATION] sensor value: %d\n", sensor_value);
	} else {
		printk("[NOTIFICATION] unexpected length %u\n", length);
	}
	total_rx_count++;

	return BT_GATT_ITER_CONTINUE;
}

static uint8_t discover_func(struct bt_conn *conn,
			     const struct bt_gatt_attr *attr,
			     struct bt_gatt_discover_params *params)
{
	int err;

	if (!attr) {
		printk("Discover complete\n");
		(void)memset(params, 0, sizeof(*params));
		return BT_GATT_ITER_STOP;
	}

	printk("[ATTRIBUTE] handle %u\n", attr->handle);

	if (!bt_uuid_cmp(discover_params.uuid, &vnd_uuid.uuid)) {

		memcpy(&discover_uuid, &vnd_sensor_uuid, sizeof(discover_uuid));
		discover_params.uuid = &discover_uuid.uuid;
		discover_params.start_handle = attr->handle + 1;
		discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

		err = bt_gatt_discover(conn, &discover_params);
		if (err) {
			printk("Discover failed (err %d)\n", err);
		}

	} else if (!bt_uuid_cmp(discover_params.uuid,
				&vnd_msg_uuid.uuid)) {
		//memcpy(&discover_uuid, BT_UUID_GATT_CCC, sizeof(struct bt_uuid_16));
		//discover_params.uuid = &discover_uuid.uuid;
		//discover_params.start_handle = attr->handle + 2;
		//discover_params.type = BT_GATT_DISCOVER_DESCRIPTOR;

		msg_attr_handle = bt_gatt_attr_value_handle(attr);
		msg_write_params.handle = msg_attr_handle;
		msg_ready = true;
		return BT_GATT_ITER_STOP;
	 	
	} else if (!bt_uuid_cmp(discover_params.uuid,
				&vnd_sensor_uuid.uuid)) {
		memcpy(&discover_uuid, BT_UUID_GATT_CCC, sizeof(struct bt_uuid_16));
		discover_params.uuid = &discover_uuid.uuid;
		discover_params.start_handle = attr->handle + 2;
		discover_params.type = BT_GATT_DISCOVER_DESCRIPTOR;
		subscribe_params.value_handle = bt_gatt_attr_value_handle(attr);

		err = bt_gatt_discover(conn, &discover_params);
		if (err) {
			printk("Discover failed (err %d)\n", err);
		}
	} else //if 
	//(!bt_uuid_cmp(discover_params.uuid, BT_UUID_GATT_CCC.uuid)) 
	{
		subscribe_params.notify = notify_func;
		subscribe_params.value = BT_GATT_CCC_NOTIFY;
		subscribe_params.ccc_handle = attr->handle;

		err = bt_gatt_subscribe(conn, &subscribe_params);
		if (err && err != -EALREADY) {
			printk("Subscribe failed (err %d)\n", err);
		} else {
			printk("[SUBSCRIBED]\n");
		}

		memcpy(&discover_uuid, &vnd_msg_uuid, sizeof(discover_uuid));
		discover_params.uuid = &discover_uuid.uuid;
		discover_params.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
		discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

		err = bt_gatt_discover(conn, &discover_params);
		if (err) {
			printk("Discover failed (err %d)\n", err);
		}

	}

	return BT_GATT_ITER_STOP;
}

static bool eir_found(struct bt_data *data, void *user_data)
{
	bt_addr_le_t *addr = user_data;
	int i;

	printk("[AD]: %u data_len %u\n", data->type, data->data_len);

	switch (data->type) {
	case BT_DATA_UUID128_SOME:
	case BT_DATA_UUID128_ALL:
		// check if we can fine a multiple of 16 bytes uuid (128 bits)
		if (data->data_len % 16 != 0U) {
			printk("AD malformed\n");
			return true;
		}

		for (i = 0; i < data->data_len; i += 16) {
			struct bt_conn_le_create_param *create_param;
			struct bt_le_conn_param *param;
			// const struct bt_uuid *uuid;
			// uint16_t u16;
			struct bt_uuid_128 uuid128;
			uuid128.uuid.type = BT_UUID_TYPE_128;
			int err;

			memcpy(&uuid128.val, &data->data[i], 16);
			// uuid = BT_UUID_DECLARE_16(sys_le16_to_cpu(u16));
			if (bt_uuid_cmp(&uuid128.uuid, &vnd_uuid.uuid)) {
				continue;
			}

			err = bt_le_scan_stop();
			if (err) {
				printk("Stop LE scan failed (err %d)\n", err);
				continue;
			}

			printk("Creating connection with Coded PHY support\n");
			param = BT_LE_CONN_PARAM_DEFAULT;
			create_param = BT_CONN_LE_CREATE_CONN;
			create_param->options |= BT_CONN_LE_OPT_CODED;
			err = bt_conn_le_create(addr, create_param, param,
						&default_conn);
			if (err) {
				printk("Create connection with Coded PHY support failed (err %d)\n",
				       err);

				printk("Creating non-Coded PHY connection\n");
				create_param->options &= ~BT_CONN_LE_OPT_CODED;
				err = bt_conn_le_create(addr, create_param,
							param, &default_conn);
				if (err) {
					printk("Create connection failed (err %d)\n", err);
					start_scan();
				}
			}

			return false;
		}
	}

	return true;
}


static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad)
{
	printk("[DEVICE]: %s, AD evt type %u, AD data len %u, RSSI %i\n",
	       bt_addr_le_str(addr), type, ad->len, rssi);

	/* We're only interested in legacy connectable events or
	 * possible extended advertising that are connectable.
	 */
	if (type == BT_GAP_ADV_TYPE_ADV_IND ||
	    type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND ||
	    type == BT_GAP_ADV_TYPE_EXT_ADV) {
		bt_data_parse(ad, eir_found, (void *)addr);
	}
}

static void start_scan(void)
{
	int err;

	/* Use active scanning and disable duplicate filtering to handle any
	 * devices that might update their advertising data at runtime. */
	struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_ACTIVE,
		.options    = BT_LE_SCAN_OPT_CODED,
		.interval   = BT_GAP_SCAN_FAST_INTERVAL,
		.window     = BT_GAP_SCAN_FAST_WINDOW,
	};

	err = bt_le_scan_start(&scan_param, device_found);
	if (err) {
		printk("Scanning with Coded PHY support failed (err %d)\n", err);

		printk("Scanning without Coded PHY\n");
		scan_param.options &= ~BT_LE_SCAN_OPT_CODED;
		err = bt_le_scan_start(&scan_param, device_found);
		if (err) {
			printk("Scanning failed to start (err %d)\n", err);
			return;
		}
	}

	printk("Scanning successfully started\n");
}


static void connected(struct bt_conn *conn, uint8_t conn_err)
{
	int err;

	if (conn_err) {
		printk("Failed to connect to %s (%u)\n", bt_conn_dst_str(conn), conn_err);

		bt_conn_drop(&default_conn);

		start_scan();
		return;
	}

	printk("Connected: %s\n", bt_conn_dst_str(conn));

	total_rx_count = 0U;

	if (conn == default_conn) {
		memcpy(&discover_uuid, &vnd_uuid.uuid, sizeof(discover_uuid));
		discover_params.uuid = &discover_uuid.uuid;
		discover_params.func = discover_func;
		discover_params.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
		discover_params.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
		discover_params.type = BT_GATT_DISCOVER_PRIMARY;

		err = bt_gatt_discover(default_conn, &discover_params);
		if (err) {
			printk("Discover failed(err %d)\n", err);
			return;
		}
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected: %s, reason 0x%02x %s\n", bt_conn_dst_str(conn),
	       reason, bt_hci_err_to_str(reason));

	if (default_conn != conn) {
		return;
	}

	bt_conn_drop(&default_conn);

	start_scan();
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};


#include <zephyr/storage/flash_map.h>
int main(void)
{
    const struct flash_area *image_update_partition;
	int erro;
    erro = flash_area_open(PARTITION_ID(image_update_partition), &image_update_partition);
    if (erro)
    {
        printf("failed to open image_update_partition\n");
        return 1;
    }
    else
    {
        printf("image_update_partition is open.\n");
    }
	
	/*
	static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
	gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);

	while (1) {
		gpio_pin_toggle_dt(&led);
		k_msleep(500);
	}
		*/
	int err;
	printf("running bluetooth\n");
	err = bt_enable(NULL);
    if (err) {
        printf("Bluetooth init failed (err %d)\n", err);
        return err;
    }
	start_scan();
	snprintk(msg, MSG_MAX_LEN, "hello");

	// main loop
	while(true){
		k_sleep(K_SECONDS(1));

		if(default_conn ){
			if(msg_ready)	bt_gatt_write(default_conn, &msg_write_params);

			// initiating firmware update
			if(start_firmware_update){
				start_firmware_update = 0;
			}
		}	
	}
	
	return 0;
}
