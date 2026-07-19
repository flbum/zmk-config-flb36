/* Copyright (c) 2026
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/bluetooth/services/bas.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>

#include <hal/nrf_power.h>
#include <zmk/battery.h>

static struct k_work_delayable charge_status_work;

static void update_charge_status(struct k_work *work) {
    uint8_t level = zmk_battery_state_of_charge();
    bool usb_powered = nrf_power_usbregstatus_vbusdet_get(NRF_POWER);
    uint8_t encoded = level & 0xfe;

    if (usb_powered) {
        encoded = level >= 99 ? 99 : encoded | 1;
    }

    if (bt_bas_get_battery_level() != encoded) {
        bt_bas_set_battery_level(encoded);
    }

    k_work_reschedule(&charge_status_work, K_SECONDS(1));
}

static int charge_status_init(void) {
    k_work_init_delayable(&charge_status_work, update_charge_status);
    k_work_schedule(&charge_status_work, K_SECONDS(1));
    return 0;
}

SYS_INIT(charge_status_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
