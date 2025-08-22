//
// Created by Sidney on 05/07/2025.
//

#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#include <iterator>

// Top to bottom
constexpr uint32_t keys_pins_rows[] = { 6, 7, 8 };
// Left to right
constexpr uint32_t keys_pins_cols[] = { 5, 4, 3 };

constexpr size_t num_key_rows = std::size(keys_pins_rows);
constexpr size_t num_key_cols = std::size(keys_pins_cols);

constexpr uint16_t display_width = 128;
constexpr uint16_t display_height = 32;

constexpr uint16_t display_left_third = display_width / 3;
constexpr uint16_t display_right_third = display_width - (display_width / 3);

constexpr uint16_t display_top_third = display_height / 3;
constexpr uint16_t display_bottom_third = display_height - (display_height / 3);

constexpr uint32_t i2c_pin_sda = 16;
constexpr uint32_t i2c_pin_scl = 17;

constexpr uint32_t analog_pin_x = 26;
constexpr uint32_t analog_pin_y = 27;

constexpr uint32_t font_height = 8;
constexpr uint32_t font_width = 5;

#endif //CONFIG_H
