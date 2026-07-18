/*
 * Copyright (c) 2026
 *
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>

static const struct device *const i2c_bus = DEVICE_DT_GET(DT_NODELABEL(i2c0));
static struct k_work_delayable test_work;

static int write_command(uint8_t command) {
    const uint8_t packet[] = {0x00, command};
    return i2c_write(i2c_bus, packet, sizeof(packet), 0x3c);
}

static int write_checkerboard(void) {
    uint8_t packet[17];
    packet[0] = 0x40;

    for (uint8_t page = 0; page < 4; page++) {
        if (write_command(0xb0 | page) < 0 || write_command(0x00) < 0 ||
            write_command(0x10) < 0) {
            return -EIO;
        }

        for (uint8_t chunk = 0; chunk < 8; chunk++) {
            for (uint8_t i = 0; i < 16; i++) {
                packet[i + 1] = ((chunk * 16 + i + page) & 1) ? 0xaa : 0x55;
            }

            if (i2c_write(i2c_bus, packet, sizeof(packet), 0x3c) < 0) {
                return -EIO;
            }
        }
    }

    return write_command(0xaf);
}

static void test_handler(struct k_work *work) {
    write_command(0xae);
    write_command(0x20);
    write_command(0x02);
    write_checkerboard();
    k_work_reschedule(&test_work, K_SECONDS(1));
}

static int flb36_i2c_test_init(void) {
    if (!device_is_ready(i2c_bus)) {
        return -ENODEV;
    }

    k_work_init_delayable(&test_work, test_handler);
    k_work_schedule(&test_work, K_MSEC(2000));
    return 0;
}

SYS_INIT(flb36_i2c_test_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
