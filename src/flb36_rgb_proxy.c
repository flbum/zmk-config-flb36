/* Copyright (c) 2026
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT flb36_behavior_rgb_proxy

#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/util.h>

#include <drivers/behavior.h>
#include <dt-bindings/zmk/rgb.h>
#include <zmk/keymap.h>

#if IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW)
#include <zmk/rgb_underglow.h>
#endif

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
static const struct behavior_parameter_value_metadata commands[] = {
    {.display_name = "Toggle On/Off", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = RGB_TOG_CMD},
    {.display_name = "Turn On", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = RGB_ON_CMD},
    {.display_name = "Turn Off", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = RGB_OFF_CMD},
    {.display_name = "Hue Up", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = RGB_HUI_CMD},
    {.display_name = "Hue Down", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = RGB_HUD_CMD},
    {.display_name = "Saturation Up", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = RGB_SAI_CMD},
    {.display_name = "Saturation Down", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = RGB_SAD_CMD},
    {.display_name = "Brightness Up", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = RGB_BRI_CMD},
    {.display_name = "Brightness Down", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = RGB_BRD_CMD},
    {.display_name = "Speed Up", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = RGB_SPI_CMD},
    {.display_name = "Speed Down", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = RGB_SPD_CMD},
    {.display_name = "Next Effect", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = RGB_EFF_CMD},
    {.display_name = "Previous Effect", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = RGB_EFR_CMD},
};

static const struct behavior_parameter_metadata_set command_set = {
    .param1_values = commands,
    .param1_values_len = ARRAY_SIZE(commands),
};

static const struct behavior_parameter_metadata metadata = {
    .sets = &command_set,
    .sets_len = 1,
};
#endif

static int pressed(struct zmk_behavior_binding *binding,
                   struct zmk_behavior_binding_event event) {
#if IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW)
    switch (binding->param1) {
    case RGB_TOG_CMD: return zmk_rgb_underglow_toggle();
    case RGB_ON_CMD: return zmk_rgb_underglow_on();
    case RGB_OFF_CMD: return zmk_rgb_underglow_off();
    case RGB_HUI_CMD: return zmk_rgb_underglow_change_hue(1);
    case RGB_HUD_CMD: return zmk_rgb_underglow_change_hue(-1);
    case RGB_SAI_CMD: return zmk_rgb_underglow_change_sat(1);
    case RGB_SAD_CMD: return zmk_rgb_underglow_change_sat(-1);
    case RGB_BRI_CMD: return zmk_rgb_underglow_change_brt(1);
    case RGB_BRD_CMD: return zmk_rgb_underglow_change_brt(-1);
    case RGB_SPI_CMD: return zmk_rgb_underglow_change_spd(1);
    case RGB_SPD_CMD: return zmk_rgb_underglow_change_spd(-1);
    case RGB_EFF_CMD: return zmk_rgb_underglow_cycle_effect(1);
    case RGB_EFR_CMD: return zmk_rgb_underglow_cycle_effect(-1);
    default: return -ENOTSUP;
    }
#else
    return 0;
#endif
}

static int released(struct zmk_behavior_binding *binding,
                    struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api api = {
    .binding_pressed = pressed,
    .binding_released = released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &metadata,
#endif
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &api);
