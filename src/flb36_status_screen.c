/*
 * Copyright (c) 2026
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#include <lvgl.h>

#include <zmk/ble.h>
#include <zmk/display.h>
#include <zmk/display/status_screen.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/split/central.h>

struct flb36_status_state {
    uint8_t left_battery;
    uint8_t right_battery;
    uint8_t active_profile;
};

static lv_obj_t *battery_label;
static lv_obj_t *profile_label;

static struct flb36_status_state get_status(const zmk_event_t *eh) {
    struct flb36_status_state state = {
        .left_battery = 0,
        .right_battery = 0,
        .active_profile = zmk_ble_active_profile_index(),
    };

    zmk_split_central_get_peripheral_battery_level(0, &state.left_battery);
    zmk_split_central_get_peripheral_battery_level(1, &state.right_battery);

    return state;
}

static void update_status(struct flb36_status_state state) {
    char batteries[24];
    char profile[16];

    snprintf(batteries, sizeof(batteries), "L %3u%%  R %3u%%", state.left_battery,
             state.right_battery);
    snprintf(profile, sizeof(profile), "PROFILE %u", state.active_profile + 1);

    lv_label_set_text(battery_label, batteries);
    lv_label_set_text(profile_label, profile);
}

ZMK_DISPLAY_WIDGET_LISTENER(flb36_status, struct flb36_status_state, update_status, get_status)
ZMK_SUBSCRIPTION(flb36_status, zmk_peripheral_battery_state_changed);
ZMK_SUBSCRIPTION(flb36_status, zmk_ble_active_profile_changed);

lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *screen = lv_obj_create(NULL);

    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(screen, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(screen, 0, LV_PART_MAIN);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    battery_label = lv_label_create(screen);
    lv_obj_set_style_text_color(battery_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(battery_label, LV_ALIGN_TOP_MID, 0, 0);

    profile_label = lv_label_create(screen);
    lv_obj_set_style_text_color(profile_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(profile_label, LV_ALIGN_BOTTOM_MID, 0, 0);

    flb36_status_init();
    lv_obj_invalidate(screen);
    return screen;
}
