/* Copyright (c) 2026
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/init.h>
#include <zephyr/kernel.h>

#include <zmk/activity.h>
#include <zmk/event_manager.h>
#include <zmk/events/activity_state_changed.h>
#include <zmk/rgb_underglow.h>

#define SUNRISE_STEPS 20

static struct k_work_delayable sunrise_work;
static struct k_work_delayable wake_on_work;
static uint8_t sunrise_step;

static void sunrise(struct k_work *work) {
    if (sunrise_step == 0) {
        zmk_rgb_underglow_select_effect(0);
        zmk_rgb_underglow_on();
    }

    zmk_rgb_underglow_set_hsb((struct zmk_led_hsb){
        .h = sunrise_step * 2,
        .s = 100 - sunrise_step,
        .b = 5 + sunrise_step,
    });

    if (sunrise_step++ < SUNRISE_STEPS) {
        k_work_reschedule(&sunrise_work, K_MSEC(100));
    } else {
        zmk_rgb_underglow_select_effect(0);
    }
}

static void wake_on(struct k_work *work) { zmk_rgb_underglow_on(); }

static int activity_listener(const zmk_event_t *eh) {
    const struct zmk_activity_state_changed *event = as_zmk_activity_state_changed(eh);

    if (event && event->state == ZMK_ACTIVITY_ACTIVE) {
        k_work_reschedule(&wake_on_work, K_MSEC(100));
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(flb36_sunrise, activity_listener);
ZMK_SUBSCRIPTION(flb36_sunrise, zmk_activity_state_changed);

static int sunrise_init(void) {
    k_work_init_delayable(&sunrise_work, sunrise);
    k_work_init_delayable(&wake_on_work, wake_on);
    k_work_schedule(&sunrise_work, K_MSEC(1500));
    return 0;
}

SYS_INIT(sunrise_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
