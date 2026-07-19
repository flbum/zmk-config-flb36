/* Copyright (c) 2026
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>

#include <zmk/ble.h>
#include <zmk/split/central.h>

#define OLED_ADDRESS 0x3c
#define OLED_WIDTH 128
#define OLED_PAGES 4

static const struct device *const i2c_bus = DEVICE_DT_GET(DT_NODELABEL(i2c0));
static struct k_work_delayable refresh_work;
static uint8_t framebuffer[OLED_WIDTH * OLED_PAGES];

static const uint8_t font[][5] = {
    [' ' - ' '] = {0, 0, 0, 0, 0},
    ['%' - ' '] = {0x62, 0x64, 0x08, 0x13, 0x23},
    ['+' - ' '] = {0x08, 0x08, 0x3e, 0x08, 0x08},
    ['-' - ' '] = {0x08, 0x08, 0x08, 0x08, 0x08},
    ['0' - ' '] = {0x3e, 0x51, 0x49, 0x45, 0x3e},
    ['1' - ' '] = {0x00, 0x42, 0x7f, 0x40, 0x00},
    ['2' - ' '] = {0x42, 0x61, 0x51, 0x49, 0x46},
    ['3' - ' '] = {0x21, 0x41, 0x45, 0x4b, 0x31},
    ['4' - ' '] = {0x18, 0x14, 0x12, 0x7f, 0x10},
    ['5' - ' '] = {0x27, 0x45, 0x45, 0x45, 0x39},
    ['6' - ' '] = {0x3c, 0x4a, 0x49, 0x49, 0x30},
    ['7' - ' '] = {0x01, 0x71, 0x09, 0x05, 0x03},
    ['8' - ' '] = {0x36, 0x49, 0x49, 0x49, 0x36},
    ['9' - ' '] = {0x06, 0x49, 0x49, 0x29, 0x1e},
    [':' - ' '] = {0x00, 0x36, 0x36, 0x00, 0x00},
    ['B' - ' '] = {0x7f, 0x49, 0x49, 0x49, 0x36},
    ['E' - ' '] = {0x7f, 0x49, 0x49, 0x49, 0x41},
    ['L' - ' '] = {0x7f, 0x40, 0x40, 0x40, 0x40},
    ['O' - ' '] = {0x3e, 0x41, 0x41, 0x41, 0x3e},
    ['R' - ' '] = {0x7f, 0x09, 0x19, 0x29, 0x46},
    ['T' - ' '] = {0x01, 0x01, 0x7f, 0x01, 0x01},
    ['Y' - ' '] = {0x03, 0x04, 0x78, 0x04, 0x03},
};

static int command(uint8_t value) {
    const uint8_t packet[] = {0x00, value};
    return i2c_write(i2c_bus, packet, sizeof(packet), OLED_ADDRESS);
}

static void draw_char(uint8_t x, uint8_t page, char c) {
    if (page >= OLED_PAGES || x + 5 >= OLED_WIDTH || c < ' ' || c > 'T') {
        return;
    }
    const uint8_t *glyph = font[c - ' '];
    for (uint8_t i = 0; i < 5; i++) {
        framebuffer[page * OLED_WIDTH + x + i] = glyph[i];
    }
}

static void draw_text(uint8_t x, uint8_t page, const char *text) {
    while (*text && x + 5 < OLED_WIDTH) {
        draw_char(x, page, *text++);
        x += 6;
    }
}

static void draw_text_2x(uint8_t x, const char *text) {
    while (*text && x + 10 < OLED_WIDTH) {
        char c = *text++;
        if (c >= ' ' && c <= 'T') {
            const uint8_t *glyph = font[c - ' '];
            for (uint8_t column = 0; column < 5; column++) {
                for (uint8_t row = 0; row < 7; row++) {
                    if (glyph[column] & BIT(row)) {
                        uint8_t px = x + column * 2;
                        uint8_t py = row * 2;
                        framebuffer[(py / 8) * OLED_WIDTH + px] |= BIT(py % 8);
                        framebuffer[(py / 8) * OLED_WIDTH + px + 1] |= BIT(py % 8);
                        framebuffer[((py + 1) / 8) * OLED_WIDTH + px] |= BIT((py + 1) % 8);
                        framebuffer[((py + 1) / 8) * OLED_WIDTH + px + 1] |= BIT((py + 1) % 8);
                    }
                }
            }
        }
        x += 12;
    }
}

static void draw_text_3x(uint8_t x, uint8_t y, const char *text) {
    while (*text && x + 15 < OLED_WIDTH) {
        char c = *text++;
        if (c >= ' ' && c <= 'Y') {
            const uint8_t *glyph = font[c - ' '];
            for (uint8_t column = 0; column < 5; column++) {
                for (uint8_t row = 0; row < 7; row++) {
                    if (!(glyph[column] & BIT(row))) {
                        continue;
                    }
                    for (uint8_t dx = 0; dx < 3; dx++) {
                        for (uint8_t dy = 0; dy < 3; dy++) {
                            uint8_t px = x + column * 3 + dx;
                            uint8_t py = y + row * 3 + dy;
                            framebuffer[(py / 8) * OLED_WIDTH + px] |= BIT(py % 8);
                        }
                    }
                }
            }
        }
        x += 18;
    }
}

static int flush(void) {
    uint8_t packet[17];
    packet[0] = 0x40;
    for (uint8_t page = 0; page < OLED_PAGES; page++) {
        if (command(0xb0 | page) < 0 || command(0x00) < 0 || command(0x10) < 0) {
            return -EIO;
        }
        for (uint8_t chunk = 0; chunk < 8; chunk++) {
            memcpy(&packet[1], &framebuffer[page * OLED_WIDTH + chunk * 16], 16);
            if (i2c_write(i2c_bus, packet, sizeof(packet), OLED_ADDRESS) < 0) {
                return -EIO;
            }
        }
    }
    return 0;
}

static void refresh(struct k_work *work) {
    uint8_t left_raw = 0, right_raw = 0;
    uint8_t left, right;
    bool left_powered, right_powered;
    char line[20];

    zmk_split_central_get_peripheral_battery_level(0, &left_raw);
    zmk_split_central_get_peripheral_battery_level(1, &right_raw);
    left_powered = left_raw & 1;
    right_powered = right_raw & 1;
    left = left_powered && left_raw == 99 ? 100 : left_raw & 0xfe;
    right = right_powered && right_raw == 99 ? 100 : right_raw & 0xfe;
    memset(framebuffer, 0, sizeof(framebuffer));

    snprintf(line, sizeof(line), "%u%%%s", left, left_powered ? "+" : "");
    draw_text_2x(0, line);
    snprintf(line, sizeof(line), "%u%%%s", right, right_powered ? "+" : "");
    draw_text_2x(128 - (strlen(line) * 12), line);
    snprintf(line, sizeof(line), "BT: %u", zmk_ble_active_profile_index() + 1);
    draw_text((128 - (strlen(line) * 6)) / 2, 3, line);
    flush();
    command(0xaf);
    k_work_reschedule(&refresh_work, K_SECONDS(1));
}

static int status_screen_init(void) {
    if (!device_is_ready(i2c_bus)) {
        return -ENODEV;
    }
    k_msleep(1200);
    const uint8_t init[] = {0xae, 0xd5, 0x80, 0xa8, 0x1f, 0xd3, 0x00, 0x40, 0x8d, 0x14,
                            0x20, 0x02, 0xa1, 0xc8, 0xda, 0x02, 0x81, 0x7f, 0xd9, 0x22,
                            0xdb, 0x20, 0xa4, 0xa6};
    for (size_t i = 0; i < sizeof(init); i++) {
        if (command(init[i]) < 0) {
            return -EIO;
        }
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    draw_text_3x(19, 5, "LEROY");
    flush();
    command(0xaf);
    k_msleep(3000);
    k_work_init_delayable(&refresh_work, refresh);
    k_work_schedule(&refresh_work, K_NO_WAIT);
    return 0;
}

SYS_INIT(status_screen_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
