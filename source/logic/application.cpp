//
// Created by Sidney on 22/08/2025.
//

#include <config.h>
#include <cstring>
#include <gui/drawing.h>
#include <usb/usb_descriptor.h>
#include <pico/bootrom.h>
#include <ff.h>
#include <tiny-json.h>
#include "application.h"

void application::init()
{
	i2c_inst_t *i2c = i2c0;
	i2c_init(i2c, 400 * 1000);

	gpio_set_function(i2c_pin_sda, GPIO_FUNC_I2C);
	gpio_set_function(i2c_pin_scl, GPIO_FUNC_I2C);

	gpio_pull_up(i2c_pin_sda);
	gpio_pull_up(i2c_pin_scl);

	m_display.init(i2c, display_width, display_height, 0x3c);
	m_display.clear();
	m_display.update();

	m_keymatrix.init(keys_pins_rows, keys_pins_cols);
	m_analogstick.init(analog_pin_x, analog_pin_y);

	m_previous_analog_x = m_analogstick.get_x_value(display_width);
	m_previous_analog_y = m_analogstick.get_y_value(display_height);

	load_configuration();

	m_last_input = to_ms_since_boot(get_absolute_time());
	m_needs_redraw = true;
}

void application::usb_ejected()
{
	m_next_state = state_t::keypad;
}


void application::load_configuration()
{
	for(auto &key : m_keymaps)
		delete key;

	m_keymaps.clear();


	parse_configuration();
	m_keymaps.push_back(build_system_keymap());

	m_current_keymap = 0;
	m_needs_redraw = true;
}

void application::parse_configuration()
{
	if(m_config_data)
	{
		delete[] m_config_data;
		m_config_data = nullptr;
	}

	FIL file;

	FILINFO info;
	if(f_stat("/config.json", &info) != FR_OK)
		return;

	if(f_open(&file, "/config.json", FA_OPEN_EXISTING | FA_READ) == FR_OK)
	{
		m_config_data = new char[info.fsize];
		if(!m_config_data)
			return;

		size_t pool_size = 512;
		constexpr size_t max_pool_size = (100 * 1024) / sizeof(json_t); // Max 100kb of RAM

		while(pool_size < max_pool_size)
		{
			json_t *pool = new json_t[pool_size];
			if(!pool)
				break;

			UINT read;
			if(f_read(&file, m_config_data, info.fsize, &read) != FR_OK)
			{
				delete[] pool;
				break;
			}

			const json_t *parent = json_create(m_config_data, pool, pool_size);
			if(!parent)
			{
				delete[] pool;
				pool_size = pool_size + 512;
			}

			if(json_getType(parent) != JSON_ARRAY)
			{
				delete[] pool;
				break;
			}

			for(const json_t *keymap = json_getChild(parent); keymap; keymap = json_getSibling(keymap))
			{
				keymap_t *result = parse_keymap(keymap);
				if(!result)
					continue;

				m_keymaps.push_back(result);
			}

			delete[] pool;
			break;
		}
	}
}

bool application::update_keypad()
{
	if(m_process_input)
		process_input();

	const uint16_t stick_x = m_analogstick.get_x_value(display_width);
	const uint16_t stick_y = m_analogstick.get_y_value(display_height);

	bool has_stick_input = false;

	if(stick_x < display_left_third && m_previous_analog_x >= display_left_third)
	{
		if(m_process_input)
			keymap_cycle_layer(false);

		m_is_mod = false;
		has_stick_input = true;
	}
	else if(stick_x > display_right_third && m_previous_analog_x <= display_right_third)
	{
		if(m_process_input)
			keymap_cycle_layer(true);

		m_is_mod = false;
		has_stick_input = true;
	}
	else if(stick_y < display_top_third && m_previous_analog_y >= display_top_third)
	{
		if(m_process_input)
			keymap_cycle(false);

		m_is_mod = false;
		has_stick_input = true;
	}
	else if(stick_y > display_bottom_third && m_previous_analog_y <= display_bottom_third)
	{
		if(m_process_input)
			keymap_cycle(true);

		m_is_mod = false;
		has_stick_input = true;
	}

	m_previous_analog_x = stick_x;
	m_previous_analog_y = stick_y;

	return has_stick_input;
}

void application::update()
{
	if(m_next_state != m_state)
	{
		switch(m_next_state)
		{
			case state_t::keypad:
				load_configuration();
				usb_set_enabled_features(USB_FEATURE_HID);
				break;
			case state_t::configure:
				usb_set_enabled_features(USB_FEATURE_MSC);
				break;
		}

		m_state = m_next_state;
		m_needs_redraw = true;

		return;
	}

	m_keymatrix.update();
	m_analogstick.update();

	const uint32_t now = to_ms_since_boot(get_absolute_time());

	switch(m_state)
	{
		case state_t::keypad:
		{
			if(update_keypad() || m_keymatrix.has_state_changed())
			{
				if(tud_suspended())
					tud_remote_wakeup();

				m_last_input = now;
				m_needs_redraw = true;
			}

			// Misc logic handling and state transitions
			if(!m_process_input && m_is_screen_on)
			{
				// Give any key event 100ms to settle before enabling input again
				// Otherwise spurious events might creep in from lack of debounce
				if((now - m_last_input) >= 150)
					m_process_input = true;
			}

			break;
		}

		case state_t::configure:
			m_last_input = now;
			break;
	}

	if(m_needs_redraw)
	{
		m_needs_redraw = false;

		draw();
		m_display.update();
	}

	tud_task();
	set_display_on((now - m_last_input) <= m_screen_timeout);
}

void application::usb_state_changed()
{
	const bool is_connected = tud_connected() && !tud_suspended();

	if(is_connected == m_is_connected)
		return;

	if(is_connected && !m_is_connected)
		m_last_input = to_ms_since_boot(get_absolute_time()); // Turn the screen on using this one weird trick

	m_is_connected = is_connected;
	m_screen_timeout = m_is_connected ? SCREEN_TIMEOUT_CONNECTED_MS : SCREEN_TIMEOUT_DISCONNECTED_MS;
}

void application::set_display_on(bool display_on)
{
	if(display_on == m_is_screen_on)
		return;

	if(!display_on)
		m_process_input = false;

	m_is_screen_on = display_on;
	m_display.set_contrast(m_is_screen_on ? m_brightness : 0);
}

void application::keymap_cycle_layer(bool cycle_next)
{
	keymap_t *map = get_active_keymap();
	uint8_t layer = map->active_page;

	if(!cycle_next)
	{
		if(layer == 0)
			layer = map->layers.size() - 1;
		else
			layer = layer - 1;
	}
	else
		layer = (layer + 1) % map->layers.size();

	map->active_page = layer;
}
void application::keymap_cycle(bool cycle_next)
{
	if(!cycle_next)
	{
		if(m_current_keymap == 0)
			m_current_keymap = m_keymaps.size() - 1;
		else
			m_current_keymap = m_current_keymap - 1;
	}
	else
		m_current_keymap = (m_current_keymap + 1) % m_keymaps.size();
}

void application::process_input()
{
	hid_keyboard_report_t state = {};
	uint8_t pressed = 0;

	keymap_t *map = get_active_keymap();
	const keylayer_t &layer = map->layers[map->active_page];

	for(uint32_t i = 0; i < num_key_rows; ++ i)
	{
		for(uint32_t j = 0; j < num_key_cols; j ++)
		{
			const keymacro_t &macro = layer.macros[i * num_key_cols + j];

			if(macro.type == keymacro_t::type_t::mod)
			{
				if(macro.mod.persist)
				{
					if(m_keymatrix.has_changed_to_enabled(i, j))
						m_is_mod = !m_is_mod;
				}
				else
				{
					m_is_mod = m_keymatrix.get_state(i, j);
				}
			}
		}
	}

	for(uint32_t i = 0; i < num_key_rows; ++ i)
	{
		for(uint32_t j = 0; j < num_key_cols; j ++)
		{
			if(m_keymatrix.get_state(i, j))
			{
				const keymacro_t &macro = m_is_mod ? layer.mod_macros[i * num_key_cols + j] : layer.macros[i * num_key_cols + j];

				switch(macro.type)
				{
					case keymacro_t::type_t::hid_key:
					{
						if(pressed < 6)
						{
							state.modifier |= macro.hid_key.modifier;
							state.keycode[pressed ++] = macro.hid_key.keycode;
						}

						break;
					}

					case keymacro_t::type_t::action:
					{
						execute_action(macro.action.action);
						break;
					}

					case keymacro_t::type_t::none:
					case keymacro_t::type_t::mod:
						break;
				}

			}
		}
	}

	if(!tud_hid_ready())
		return;

	if(pressed > 0 || (pressed == 0 && m_any_key_down))
	{
		if(pressed > 0)
		{
			tud_hid_keyboard_report(REPORT_ID_KEYBOARD, state.modifier, state.keycode);
			m_any_key_down = true;
		}
		else if(m_any_key_down)
		{
			tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, nullptr);
			m_any_key_down = false;
		}
	}
}

void application::execute_action(action_t action)
{
	switch(action)
	{
		case action_t::flash:
		{
			m_display.clear();
			draw_string(&m_display, "Ready to flash!", true, 0, (display_height - font_height) / 2, display_width, text_justification_t::center);
			m_display.update();

			sleep_ms(1);
			reset_usb_boot(0, 0);

			break;
		}

		case action_t::configure:
		{
			m_next_state = state_t::configure;
			break;
		}

		case action_t::brightness_down:
		{
			if(m_brightness > 5)
				m_brightness = m_brightness - 5;
			else
				m_brightness = 5;

			m_display.set_contrast(m_brightness);
			break;
		}

		case action_t::brightness_up:
		{
			if(m_brightness <= 250)
				m_brightness = m_brightness + 5;
			else
				m_brightness = 255;

			m_display.set_contrast(m_brightness);
			break;
		}
	}
}

void application::draw()
{
	m_display.clear();

	switch(m_state)
	{
		case state_t::keypad:
			draw_active_keymap();
			break;

		case state_t::configure:
			draw_string(&m_display, "Configuring...", true, 0, (display_height - font_height) / 2, display_width, text_justification_t::center);
			break;
	}
}

void application::draw_active_keymap()
{
	keymap_t *keymap = get_active_keymap();
	const keylayer_t &layer = keymap->layers[keymap->active_page];

	{
		uint16_t offset = draw_string(&m_display, keymap->name, true, 0, 0, display_width);

		if(layer.name)
		{
			offset += 1;
			offset += draw_string(&m_display, "|", true, offset, 0, display_width) + 1;

			draw_string(&m_display, layer.name, true, offset, 0, display_width);
		}

		m_display.stroke_line_horizontal(0, font_height, display_width, true);

		if(keymap->layers.size() > 1)
		{
			char text[16];
			sprintf(text, "%d/%d", (int)(keymap->active_page + 1), (int)keymap->layers.size());

			draw_string(&m_display, text, true, 0, 0, display_width, text_justification_t::right);
		}
	}

	const uint32_t width = display_width / num_key_cols;
	const uint32_t height = (display_height - 8) / num_key_rows;

	for(uint32_t x = 0; x < num_key_cols; ++ x)
	{
		for(uint32_t y = 0; y < num_key_rows; ++ y)
		{
			const keymacro_t &macro = m_is_mod ? layer.mod_macros[y * num_key_cols + x] : layer.macros[y * num_key_cols + x];

			char string[32] = {};
			const uint8_t max_length = width / font_width;

			const bool foreground = !m_keymatrix.get_state(y, x);

			const uint32_t off_x = x * width;
			const uint32_t off_y = y * height + font_height + 2;

			m_display.fill_rect(off_x + 1, off_y + 1, width - 2, height - 2, !foreground);

			uint8_t length = 0;

			switch(macro.type)
			{
				case keymacro_t::type_t::none:
					length += strlcpy(string + length, "None", max_length - length);
					break;

				case keymacro_t::type_t::hid_key:
				{
					if(macro.hid_key.label && macro.hid_key.label[0] != '\0')
					{
						length += strlcpy(string + length, macro.hid_key.label, max_length - length);
						break;
					}

					const uint8_t modifier = macro.hid_key.modifier;
					const uint8_t keycode = macro.hid_key.keycode;

					if(modifier != 0)
					{
						if(modifier & (KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT))
							length += strlcpy(string + length, "Shf ", max_length - length);

						if(modifier & (KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_RIGHTCTRL))
							length += strlcpy(string + length, "Ctl ", max_length - length);

						if(modifier & (KEYBOARD_MODIFIER_LEFTALT | KEYBOARD_MODIFIER_RIGHTALT))
							length += strlcpy(string + length, "Alt ", max_length - length);
					}

#define MAP_DIRECT_CHAR(key) \
				case HID_KEY_##key: \
					string[length ++] = *(#key); \
					break

#define MAP_SINGLE_CHAR(key, chr) \
				case HID_KEY_##key: \
					string[length ++] = chr; \
					break

#define MAP_STRING_CHAR(key, text) \
				case HID_KEY_##key: \
					length += strlcpy(string + length, text, max_length - length); \
					break

					switch(keycode)
					{
						MAP_DIRECT_CHAR(A);
						MAP_DIRECT_CHAR(B);
						MAP_DIRECT_CHAR(C);
						MAP_DIRECT_CHAR(D);
						MAP_DIRECT_CHAR(E);
						MAP_DIRECT_CHAR(F);
						MAP_DIRECT_CHAR(G);
						MAP_DIRECT_CHAR(H);
						MAP_DIRECT_CHAR(I);
						MAP_DIRECT_CHAR(J);
						MAP_DIRECT_CHAR(K);
						MAP_DIRECT_CHAR(L);
						MAP_DIRECT_CHAR(M);
						MAP_DIRECT_CHAR(N);
						MAP_DIRECT_CHAR(O);
						MAP_DIRECT_CHAR(P);
						MAP_DIRECT_CHAR(Q);
						MAP_DIRECT_CHAR(R);
						MAP_DIRECT_CHAR(S);
						MAP_DIRECT_CHAR(T);
						MAP_DIRECT_CHAR(U);
						MAP_DIRECT_CHAR(V);
						MAP_DIRECT_CHAR(W);
						MAP_DIRECT_CHAR(X);
						MAP_DIRECT_CHAR(Y);
						MAP_DIRECT_CHAR(Z);

						MAP_DIRECT_CHAR(1);
						MAP_DIRECT_CHAR(2);
						MAP_DIRECT_CHAR(3);
						MAP_DIRECT_CHAR(4);
						MAP_DIRECT_CHAR(5);
						MAP_DIRECT_CHAR(6);
						MAP_DIRECT_CHAR(7);
						MAP_DIRECT_CHAR(8);
						MAP_DIRECT_CHAR(9);
						MAP_DIRECT_CHAR(0);

						MAP_STRING_CHAR(ENTER, "Enter");
						MAP_STRING_CHAR(ESCAPE, "ESC");
						MAP_STRING_CHAR(BACKSPACE, "Bckspce");
						MAP_STRING_CHAR(TAB, "Tab");
						MAP_STRING_CHAR(SPACE, "Space");
						MAP_SINGLE_CHAR(MINUS, '-');
						MAP_SINGLE_CHAR(EQUAL, '=');

						MAP_SINGLE_CHAR(BRACKET_LEFT, '[');
						MAP_SINGLE_CHAR(BRACKET_RIGHT, ']');

						MAP_SINGLE_CHAR(BACKSLASH, '/');
						MAP_SINGLE_CHAR(SEMICOLON, ';');
						MAP_SINGLE_CHAR(APOSTROPHE, '\'');
						MAP_SINGLE_CHAR(GRAVE, '/');
						MAP_SINGLE_CHAR(COMMA, ',');
						MAP_SINGLE_CHAR(PERIOD, '.');
						MAP_SINGLE_CHAR(SLASH, '\\');
						MAP_STRING_CHAR(CAPS_LOCK, "CAPS");

						MAP_STRING_CHAR(F1, "F1");
						MAP_STRING_CHAR(F2, "F2");
						MAP_STRING_CHAR(F3, "F3");
						MAP_STRING_CHAR(F4, "F4");
						MAP_STRING_CHAR(F5, "F5");
						MAP_STRING_CHAR(F6, "F6");
						MAP_STRING_CHAR(F7, "F7");
						MAP_STRING_CHAR(F8, "F8");
						MAP_STRING_CHAR(F9, "F9");
						MAP_STRING_CHAR(F10, "F10");
						MAP_STRING_CHAR(F11, "F11");
						MAP_STRING_CHAR(F12, "F12");

						MAP_STRING_CHAR(PRINT_SCREEN, "Prnt");
						MAP_STRING_CHAR(SCROLL_LOCK, "Scroll");

						MAP_STRING_CHAR(PAUSE, "Pause");
						MAP_STRING_CHAR(INSERT, "Insrt");
						MAP_STRING_CHAR(HOME, "Home");
						MAP_STRING_CHAR(PAGE_UP, "Pg Up");
						MAP_STRING_CHAR(DELETE, "DEL");
						MAP_STRING_CHAR(END, "End");
						MAP_STRING_CHAR(PAGE_DOWN, "Pg Down");
						MAP_STRING_CHAR(ARROW_RIGHT, "->");
						MAP_STRING_CHAR(ARROW_LEFT, "<-");
						MAP_STRING_CHAR(ARROW_DOWN, "v");
						MAP_STRING_CHAR(ARROW_UP, "^");

						MAP_STRING_CHAR(NUM_LOCK, "Nm Lck");
						MAP_STRING_CHAR(KEYPAD_DIVIDE, "/");
						MAP_SINGLE_CHAR(KEYPAD_MULTIPLY, '*');
						MAP_SINGLE_CHAR(KEYPAD_SUBTRACT, '-');
						MAP_SINGLE_CHAR(KEYPAD_ADD, '+');
						MAP_STRING_CHAR(KEYPAD_ENTER, "Enter");
						MAP_SINGLE_CHAR(KEYPAD_1, '1');
						MAP_SINGLE_CHAR(KEYPAD_2, '2');
						MAP_SINGLE_CHAR(KEYPAD_3, '3');
						MAP_SINGLE_CHAR(KEYPAD_4, '4');
						MAP_SINGLE_CHAR(KEYPAD_5, '5');
						MAP_SINGLE_CHAR(KEYPAD_6, '6');
						MAP_SINGLE_CHAR(KEYPAD_7, '7');
						MAP_SINGLE_CHAR(KEYPAD_8, '8');
						MAP_SINGLE_CHAR(KEYPAD_9, '9');
						MAP_SINGLE_CHAR(KEYPAD_0, '0');
						MAP_SINGLE_CHAR(KEYPAD_DECIMAL, '.');
						MAP_SINGLE_CHAR(KEYPAD_EQUAL, '=');

						MAP_STRING_CHAR(APPLICATION, "App");
						MAP_STRING_CHAR(POWER, "Pwr");

						MAP_STRING_CHAR(F13, "F13");
						MAP_STRING_CHAR(F14, "F14");
						MAP_STRING_CHAR(F15, "F15");
						MAP_STRING_CHAR(F16, "F16");
						MAP_STRING_CHAR(F17, "F17");
						MAP_STRING_CHAR(F18, "F18");
						MAP_STRING_CHAR(F19, "F19");
						MAP_STRING_CHAR(F20, "F20");
						MAP_STRING_CHAR(F21, "F21");
						MAP_STRING_CHAR(F22, "F22");
						MAP_STRING_CHAR(F23, "F23");
						MAP_STRING_CHAR(F24, "F24");

						MAP_SINGLE_CHAR(CURRENCY_UNIT , '$');
						MAP_SINGLE_CHAR(KEYPAD_LEFT_PARENTHESIS , '(');
						MAP_SINGLE_CHAR(KEYPAD_RIGHT_PARENTHESIS, ')');
						MAP_SINGLE_CHAR(KEYPAD_LEFT_BRACE , '{');
						MAP_SINGLE_CHAR(KEYPAD_RIGHT_BRACE, '}');
						MAP_SINGLE_CHAR(KEYPAD_PERCENT, '%');
						MAP_SINGLE_CHAR(KEYPAD_LESS_THAN, '<');
						MAP_SINGLE_CHAR(KEYPAD_GREATER_THAN, '>');
						MAP_SINGLE_CHAR(KEYPAD_AMPERSAND, '&');
						MAP_SINGLE_CHAR(KEYPAD_VERTICAL_BAR, '|');

						MAP_SINGLE_CHAR(KEYPAD_COLON, ':');
						MAP_SINGLE_CHAR(KEYPAD_HASH, '#');
						MAP_SINGLE_CHAR(KEYPAD_AT, '@');
						MAP_SINGLE_CHAR(KEYPAD_EXCLAMATION, '!');

						default:
							break;
					}

#undef MAP_STRING_CHAR
#undef MAP_SINGLE_CHAR
#undef MAP_DIRECT_CHAR

					if(length == 0)
					{
						string[length ++] = 'N';
						string[length ++] = '/';
						string[length ++] = 'A';
					}

					break;
				}
				case keymacro_t::type_t::action:
				{
					switch(macro.action.action)
					{
						case action_t::flash:
							length += strlcpy(string + length, "Flash", max_length - length);
							break;
						case action_t::configure:
							length += strlcpy(string + length, "Config", max_length - length);
							break;
						case action_t::brightness_down:
							length += strlcpy(string + length, "Brt -", max_length - length);
							break;
						case action_t::brightness_up:
							length += strlcpy(string + length, "Brt +", max_length - length);
							break;
					}

					break;
				}

				case keymacro_t::type_t::mod:
					length += strlcpy(string + length, "Mod", max_length - length);
					break;
			}

			string[length] = '\0';
			draw_string(&m_display, string,  foreground, off_x, off_y  + ((height - font_height) / 2), width, text_justification_t::center);
		}
	}
}
