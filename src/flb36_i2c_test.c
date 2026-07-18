/*
 * Copyright (c) 2026
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>

static const struct device *const i2c_bus = DEVICE_DT_GET(DT_NODELABEL(oled_i2c));
static struct k_work_delayable test_work;

static bool turn_display_off(uint16_t address) {
    const uint8_t display_off = 0xae;
    return i2c_burst_write(i2c_bus, address, 0x00, &display_off, sizeof(display_off)) == 0;
}

static void test_handler(struct k_work *work) {
    if (!turn_display_off(0x3c)) {
        turn_display_off(0x3d);
    }

    k_work_reschedule(&test_work, K_SECONDS(1));
}

static int flb36_i2c_test_init(void) {
    if (!device_is_ready(i2c_bus)) {
        return -ENODEV;
    }

    k_work_init_delayable(&test_work, test_handler);
    k_work_schedule(&test_work, K_MSEC(1500));
    return 0;
}

SYS_INIT(flb36_i2c_test_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
