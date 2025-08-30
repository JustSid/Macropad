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

keymap_t *build_system_keymap()
{
	size_t index = 0;

	keylayer_t layer;
	layer.macros[index ++] = build_action_macro(action_t::configure);
	layer.macros[index ++] = build_action_macro(action_t::brightness_up);
	layer.macros[index ++] = build_action_macro(action_t::brightness_down);

	layer.macros[index ++] = build_hid_macro(HID_KEY_Z, KEYBOARD_MODIFIER_LEFTCTRL);
	layer.macros[index ++] = build_hid_macro(HID_KEY_C, KEYBOARD_MODIFIER_LEFTCTRL);
	layer.macros[index ++] = build_hid_macro(HID_KEY_V, KEYBOARD_MODIFIER_LEFTCTRL);

	layer.macros[index ++] = build_hid_macro(HID_KEY_ESCAPE);
	layer.macros[index ++] = build_hid_macro(HID_KEY_ENTER);
	layer.macros[index ++] = build_action_macro(action_t::flash);

	keymap_t *map = new keymap_t;
	map->name = "System";
	map->active_page = 0;
	map->layers.push_back(layer);

	return map;
}

uint8_t string_to_hid_key(const char *str)
{
#define MAP_KEY(key) \
	if(strcmp(str, #key) == 0) \
		return HID_KEY_##key;

	MAP_KEY(A);
	MAP_KEY(B);
	MAP_KEY(C);
	MAP_KEY(D);
	MAP_KEY(E);
	MAP_KEY(F);
	MAP_KEY(G);
	MAP_KEY(H);
	MAP_KEY(I);
	MAP_KEY(J);
	MAP_KEY(K);
	MAP_KEY(L);
	MAP_KEY(M);
	MAP_KEY(N);
	MAP_KEY(O);
	MAP_KEY(P);
	MAP_KEY(Q);
	MAP_KEY(R);
	MAP_KEY(S);
	MAP_KEY(T);
	MAP_KEY(U);
	MAP_KEY(V);
	MAP_KEY(W);
	MAP_KEY(X);
	MAP_KEY(Y);
	MAP_KEY(Z);

	MAP_KEY(1);
	MAP_KEY(2);
	MAP_KEY(3);
	MAP_KEY(4);
	MAP_KEY(5);
	MAP_KEY(6);
	MAP_KEY(7);
	MAP_KEY(8);
	MAP_KEY(9);
	MAP_KEY(0);

	MAP_KEY(ENTER);
	MAP_KEY(ESCAPE);
	MAP_KEY(BACKSPACE);
	MAP_KEY(TAB);
	MAP_KEY(SPACE);
	MAP_KEY(MINUS);
	MAP_KEY(EQUAL);

	MAP_KEY(BRACKET_LEFT);
	MAP_KEY(BRACKET_RIGHT);

	MAP_KEY(BACKSLASH);
	MAP_KEY(SEMICOLON);
	MAP_KEY(APOSTROPHE);
	MAP_KEY(GRAVE);
	MAP_KEY(COMMA);
	MAP_KEY(PERIOD);
	MAP_KEY(SLASH);
	MAP_KEY(CAPS_LOCK);

	MAP_KEY(F1);
	MAP_KEY(F2);
	MAP_KEY(F3);
	MAP_KEY(F4);
	MAP_KEY(F5);
	MAP_KEY(F6);
	MAP_KEY(F7);
	MAP_KEY(F8);
	MAP_KEY(F9);
	MAP_KEY(F10);
	MAP_KEY(F11);
	MAP_KEY(F12);

	MAP_KEY(PRINT_SCREEN);
	MAP_KEY(SCROLL_LOCK);

	MAP_KEY(PAUSE);
	MAP_KEY(INSERT);
	MAP_KEY(HOME);
	MAP_KEY(PAGE_UP);
	MAP_KEY(DELETE);
	MAP_KEY(END);
	MAP_KEY(PAGE_DOWN);
	MAP_KEY(ARROW_RIGHT);
	MAP_KEY(ARROW_LEFT);
	MAP_KEY(ARROW_DOWN);
	MAP_KEY(ARROW_UP);

	MAP_KEY(NUM_LOCK);
	MAP_KEY(KEYPAD_DIVIDE);
	MAP_KEY(KEYPAD_MULTIPLY);
	MAP_KEY(KEYPAD_SUBTRACT);
	MAP_KEY(KEYPAD_ADD);
	MAP_KEY(KEYPAD_ENTER);
	MAP_KEY(KEYPAD_1);
	MAP_KEY(KEYPAD_2);
	MAP_KEY(KEYPAD_3);
	MAP_KEY(KEYPAD_4);
	MAP_KEY(KEYPAD_5);
	MAP_KEY(KEYPAD_6);
	MAP_KEY(KEYPAD_7);
	MAP_KEY(KEYPAD_8);
	MAP_KEY(KEYPAD_9);
	MAP_KEY(KEYPAD_0);
	MAP_KEY(KEYPAD_DECIMAL);
	MAP_KEY(KEYPAD_EQUAL);

	MAP_KEY(APPLICATION);
	MAP_KEY(POWER);

	MAP_KEY(F13);
	MAP_KEY(F14);
	MAP_KEY(F15);
	MAP_KEY(F16);
	MAP_KEY(F17);
	MAP_KEY(F18);
	MAP_KEY(F19);
	MAP_KEY(F20);
	MAP_KEY(F21);
	MAP_KEY(F22);
	MAP_KEY(F23);
	MAP_KEY(F24);

	MAP_KEY(CURRENCY_UNIT);
	MAP_KEY(KEYPAD_LEFT_PARENTHESIS);
	MAP_KEY(KEYPAD_RIGHT_PARENTHESIS);
	MAP_KEY(KEYPAD_LEFT_BRACE);
	MAP_KEY(KEYPAD_RIGHT_BRACE);
	MAP_KEY(KEYPAD_PERCENT);
	MAP_KEY(KEYPAD_LESS_THAN);
	MAP_KEY(KEYPAD_GREATER_THAN);
	MAP_KEY(KEYPAD_AMPERSAND);
	MAP_KEY(KEYPAD_VERTICAL_BAR);

	MAP_KEY(KEYPAD_COLON);
	MAP_KEY(KEYPAD_HASH);
	MAP_KEY(KEYPAD_AT);
	MAP_KEY(KEYPAD_EXCLAMATION);

#undef MAP_KEY

	return HID_KEY_NONE;
}

void parse_macros(keymacro_t *macros, keymacro_t *reference, const json_t *json)
{
	if(json_getType(json) != JSON_ARRAY)
		return;

	size_t index = 0;

	for(const json_t *entry = json_getChild(json); entry; entry = json_getSibling(entry))
	{
		if(reference && reference[index].type == keymacro_t::type_t::mod)
		{
			macros[index] = reference[index];

			if((++ index) >= num_key_cols * num_key_rows)
				return;

			continue;
		}

		const char *type = json_getPropertyValue(entry, "t");
		if(!type)
			type = "hid";

		keymacro_t macro = {};
		macro.type = keymacro_t::type_t::none;

		if(strcmp(type, "action") == 0)
			macro.type = keymacro_t::type_t::action;
		else if(strcmp(type, "mod") == 0)
		{
			const json_t *persist = json_getProperty(entry, "p");

			macro.type = keymacro_t::type_t::mod;
			macro.mod.persist = (persist && json_getBoolean(persist));
		}
		else if(strcmp(type, "hid") == 0)
		{
			macro.type = keymacro_t::type_t::hid_key;

			const char *value = json_getPropertyValue(entry, "v");
			const char *modifier = json_getPropertyValue(entry, "m");
			const char *label = json_getPropertyValue(entry, "l");

			if(modifier)
			{
				if(strstr(modifier, "lctrl"))
					macro.hid_key.modifier |= KEYBOARD_MODIFIER_LEFTCTRL;
				if(strstr(modifier, "rctrl"))
					macro.hid_key.modifier |= KEYBOARD_MODIFIER_RIGHTCTRL;

				if(strstr(modifier, "lshft"))
					macro.hid_key.modifier |= KEYBOARD_MODIFIER_LEFTSHIFT;
				if(strstr(modifier, "rshft"))
					macro.hid_key.modifier |= KEYBOARD_MODIFIER_RIGHTSHIFT;

				if(strstr(modifier, "lalt"))
					macro.hid_key.modifier |= KEYBOARD_MODIFIER_LEFTALT;
				if(strstr(modifier, "ralt"))
					macro.hid_key.modifier |= KEYBOARD_MODIFIER_RIGHTALT;
			}

			if(value)
				macro.hid_key.keycode = string_to_hid_key(value);

			macro.hid_key.label = label;
		}

		macros[index ++] = macro;

		if(index >= num_key_cols * num_key_rows)
			return;
	}
}

keymap_t *parse_keymap(const json_t *keymap)
{
	if(json_getType(keymap) != JSON_OBJ)
		return nullptr;

	const json_t *layers = json_getProperty(keymap, "layers");
	if(!layers || json_getType(layers) != JSON_ARRAY)
		return nullptr;

	keymap_t *result = new keymap_t;
	result->name = json_getPropertyValue(keymap, "name");
	result->active_page = 0;

	if(!result->name)
		result->name = "No name";

	for(const json_t *layer = json_getChild(layers); layer; layer = json_getSibling(layer))
	{
		const json_t *base = json_getProperty(layer, "base");
		if(!base)
			continue;

		keylayer_t parsed;
		parse_macros(parsed.macros, nullptr, base);

		const json_t *mod = json_getProperty(layer, "mod");
		if(mod)
			parse_macros(parsed.mod_macros, parsed.macros, mod);

		const json_t *name = json_getProperty(layer, "name");
		if(name && json_getType(name) == JSON_TEXT)
			parsed.name = json_getValue(name);

		result->layers.push_back(parsed);
	}

	if(result->layers.empty())
	{
		delete result;
		return nullptr;
	}

	return result;
}
