//
// Created by Sidney on 22/08/2025.
//

#ifndef MACROPAD_APPLICATION_H
#define MACROPAD_APPLICATION_H

#include "../devices/display.h"
#include "../devices/keymatrix.h"
#include "../devices/analogstick.h"

#include "keylayer.h"

#define SCREEN_TIMEOUT_CONNECTED_MS     (15 * 60 * 1000)
#define SCREEN_TIMEOUT_DISCONNECTED_MS  (10 * 1000)

class application
{
public:
	application() = default;

	void init();
	void update();

	void usb_state_changed();

private:
	void set_display_on(bool display_on);

	void draw_active_keymap();

	void process_input();
	void execute_action(action_t action);

	keymap_t *get_active_keymap() const { return m_keymaps[m_current_keymap]; }

	void keymap_cycle_layer(bool cycle_next);
	void keymap_cycle(bool cycle_next);

	display_t m_display;
	keymatrix_t m_keymatrix;
	analogstick_t m_analogstick;

	uint16_t m_previous_analog_x;
	uint16_t m_previous_analog_y;

	bool m_is_mod = false;
	bool m_any_key_down = false;
	bool m_needs_redraw = false;

	bool m_is_connected = false;
	bool m_process_input = true;

	uint8_t m_brightness = 5;
	uint32_t m_last_input;

	bool m_is_screen_on = false;
	uint32_t m_screen_timeout = SCREEN_TIMEOUT_DISCONNECTED_MS;

	std::vector<keymap_t *> m_keymaps;
	size_t m_current_keymap = 0;
};

#endif //MACROPAD_APPLICATION_H