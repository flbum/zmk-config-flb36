/*
 * Copyright (c) 2026
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>

enum test_result {
    TEST_WAITING,
    TEST_ADDRESS_3C,
    TEST_ADDRESS_3D,
    TEST_NO_DEVICE,
};

static const struct device *const i2c_bus = DEVICE_DT_GET(DT_NODELABEL(i2c0));
static const struct gpio_dt_spec status_led =
    GPIO_DT_SPEC_GET(DT_NODELABEL(blue_led), gpios);

static enum test_result result = TEST_WAITING;
static bool led_on;
static struct k_work_delayable test_work;

static bool address_responds(uint16_t address) {
    const uint8_t control_byte = 0x00;
    return i2c_write(i2c_bus, &control_byte, sizeof(control_byte), address) == 0;
}

static void test_handler(struct k_work *work) {
    if (result == TEST_WAITING) {
        if (address_responds(0x3c)) {
            result = TEST_ADDRESS_3C;
            gpio_pin_set_dt(&status_led, 1);
            return;
        }

        result = address_responds(0x3d) ? TEST_ADDRESS_3D : TEST_NO_DEVICE;
    }

    led_on = !led_on;
    gpio_pin_set_dt(&status_led, led_on);

    k_work_reschedule(&test_work,
                      result == TEST_ADDRESS_3D ? K_MSEC(750) : K_MSEC(125));
}

static int flb36_i2c_test_init(void) {
    if (!device_is_ready(i2c_bus) || !gpio_is_ready_dt(&status_led)) {
        return -ENODEV;
    }

    int err = gpio_pin_configure_dt(&status_led, GPIO_OUTPUT_INACTIVE);
    if (err) {
        return err;
    }

    k_work_init_delayable(&test_work, test_handler);
    k_work_schedule(&test_work, K_MSEC(1500));
    return 0;
}

SYS_INIT(flb36_i2c_test_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
