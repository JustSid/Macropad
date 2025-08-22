//
// Created by Sidney on 04/07/2025.
//

#include <tusb.h>
#include "keylayer.h"

keymacro_t build_none_macro()
{
	keymacro_t macro;
	macro.type = keymacro_t::type_t::none;

	return macro;
}

keymacro_t build_hid_macro(uint8_t keycode, uint8_t modifier = 0)
{
	keymacro_t macro;
	macro.type = keymacro_t::type_t::hid_key;
	macro.hid_key.modifier = modifier;
	macro.hid_key.keycode = keycode;

	return macro;
}

keymacro_t build_action_macro(action_t action)
{
	keymacro_t macro;
	macro.type = keymacro_t::type_t::action;
	macro.action.action = action;

	return macro;
}

keymacro_t build_mod_macro(bool toggle)
{
	keymacro_t macro;
	macro.type = keymacro_t::type_t::mod;
	macro.mod.persist = toggle;

	return macro;
}

keymap_t *build_blender_keymap()
{
	keymap_t *map = new keymap_t;
	map->title = "Blender";
	map->active_page = 0;
	map->layers.push_back({
		.macros = {
		{ build_mod_macro(false), build_hid_macro(HID_KEY_G), build_hid_macro(HID_KEY_R) },
		{ build_hid_macro(HID_KEY_X), build_hid_macro(HID_KEY_Y), build_hid_macro(HID_KEY_Z) }
		},
		 .mod_macros = {
			{ build_mod_macro(false), build_hid_macro(HID_KEY_NONE, KEYBOARD_MODIFIER_LEFTCTRL), build_hid_macro(HID_KEY_NONE, KEYBOARD_MODIFIER_LEFTSHIFT) },
			{ build_hid_macro(HID_KEY_A), build_hid_macro(HID_KEY_TAB), build_hid_macro(HID_KEY_A, KEYBOARD_MODIFIER_LEFTSHIFT) }
		}
	});
	map->layers.push_back({ .macros ={
		{ build_hid_macro(HID_KEY_A, KEYBOARD_MODIFIER_LEFTSHIFT), build_hid_macro(HID_KEY_NONE, KEYBOARD_MODIFIER_LEFTSHIFT), build_hid_macro(HID_KEY_NONE, KEYBOARD_MODIFIER_LEFTCTRL) },
		{ build_hid_macro(HID_KEY_A), build_hid_macro(HID_KEY_D, KEYBOARD_MODIFIER_LEFTSHIFT), build_hid_macro(HID_KEY_H) }
	} });

	return map;
}

keymap_t *build_fusion_keymap()
{
	keymap_t *map = new keymap_t;
	map->title = "Fusion 360";
	map->active_page = 0;
	map->layers.push_back({ .macros = {
		{ build_hid_macro(HID_KEY_ESCAPE), build_hid_macro(HID_KEY_DELETE), build_hid_macro(HID_KEY_Z, KEYBOARD_MODIFIER_LEFTCTRL) },
		{ build_hid_macro(HID_KEY_C, KEYBOARD_MODIFIER_LEFTCTRL), build_hid_macro(HID_KEY_V, KEYBOARD_MODIFIER_LEFTCTRL), build_hid_macro(HID_KEY_Y, KEYBOARD_MODIFIER_LEFTCTRL)  },
		{ build_hid_macro(HID_KEY_X), build_hid_macro(HID_KEY_P), build_hid_macro(HID_KEY_D) }
	} });

	return map;
}

keymap_t *build_system_keymap()
{
	keymap_t *map = new keymap_t;
	map->title = "System";
	map->active_page = 0;
	map->layers.push_back({
		.macros = {
			{ build_action_macro(action_t::flash), build_action_macro(action_t::brightness_up), build_action_macro(action_t::brightness_down) },
			{ build_hid_macro(HID_KEY_Z, KEYBOARD_MODIFIER_LEFTCTRL), build_hid_macro(HID_KEY_C, KEYBOARD_MODIFIER_LEFTCTRL), build_hid_macro(HID_KEY_V, KEYBOARD_MODIFIER_LEFTCTRL) },
			{ build_hid_macro(HID_KEY_ESCAPE), build_hid_macro(HID_KEY_ENTER), build_hid_macro(HID_KEY_Y, KEYBOARD_MODIFIER_LEFTCTRL) }
		}
	});
	map->layers.push_back({
		.macros = {
			{ build_hid_macro(HID_KEY_7), build_hid_macro(HID_KEY_8), build_hid_macro(HID_KEY_9) },
			{ build_hid_macro(HID_KEY_4), build_hid_macro(HID_KEY_5), build_hid_macro(HID_KEY_6) },
			{ build_hid_macro(HID_KEY_1), build_hid_macro(HID_KEY_2), build_hid_macro(HID_KEY_3) },
		}
	});

	return map;
}
