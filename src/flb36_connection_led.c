/*
 * Copyright (c) 2026
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/bluetooth/conn.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>

#define REQUIRED_PERIPHERALS 2

static const struct gpio_dt_spec blue_led =
    GPIO_DT_SPEC_GET(DT_NODELABEL(blue_led), gpios);

static void count_central_links(struct bt_conn *conn, void *user_data) {
    struct bt_conn_info info;
    uint8_t *count = user_data;

    if (bt_conn_get_info(conn, &info) == 0 && info.state == BT_CONN_STATE_CONNECTED &&
        info.role == BT_CONN_ROLE_CENTRAL) {
        (*count)++;
    }
}

static void update_blue_led(struct k_work *work) {
    uint8_t connected_peripherals = 0;

    bt_conn_foreach(BT_CONN_TYPE_LE, count_central_links, &connected_peripherals);
    gpio_pin_set_dt(&blue_led, connected_peripherals >= REQUIRED_PERIPHERALS);
}

K_WORK_DEFINE(update_blue_led_work, update_blue_led);

static void connection_changed(struct bt_conn *conn, uint8_t err) {
    k_work_submit(&update_blue_led_work);
}

static void disconnection_changed(struct bt_conn *conn, uint8_t reason) {
    k_work_submit(&update_blue_led_work);
}

BT_CONN_CB_DEFINE(flb36_connection_callbacks) = {
    .connected = connection_changed,
    .disconnected = disconnection_changed,
};

static int flb36_connection_led_init(void) {
    if (!gpio_is_ready_dt(&blue_led)) {
        return -ENODEV;
    }

    int err = gpio_pin_configure_dt(&blue_led, GPIO_OUTPUT_INACTIVE);
    if (err) {
        return err;
    }

    k_work_submit(&update_blue_led_work);
    return 0;
}

SYS_INIT(flb36_connection_led_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
