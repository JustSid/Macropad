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
	layer.macros[index ++] = build_action_macro(action_t::flash);
	layer.macros[index ++] = build_action_macro(action_t::brightness_up);
	layer.macros[index ++] = build_action_macro(action_t::brightness_down);

	layer.macros[index ++] = build_hid_macro(HID_KEY_Z, KEYBOARD_MODIFIER_LEFTCTRL);
	layer.macros[index ++] = build_hid_macro(HID_KEY_C, KEYBOARD_MODIFIER_LEFTCTRL);
	layer.macros[index ++] = build_hid_macro(HID_KEY_V, KEYBOARD_MODIFIER_LEFTCTRL);

	layer.macros[index ++] = build_hid_macro(HID_KEY_ESCAPE);
	layer.macros[index ++] = build_hid_macro(HID_KEY_ENTER);
	layer.macros[index ++] = build_hid_macro(HID_KEY_Y, KEYBOARD_MODIFIER_LEFTCTRL);

	keymap_t *map = new keymap_t;
	map->title = "System";
	map->active_page = 0;
	map->layers.push_back(layer);

	return map;
}

void parse_macros(keymacro_t *macros, const json_t *json)
{
	if(json_getType(json) != JSON_ARRAY)
		return;

	size_t index = 0;

	for(const json_t *entry = json_getChild(json); entry; entry = json_getSibling(entry))
	{
		const char *type = json_getPropertyValue(entry, "t");
		if(!type)
			type = "hid";

		keymacro_t macro = {};
		macro.type = keymacro_t::type_t::none;

		if(strcmp(type, "action") == 0)
			macro.type = keymacro_t::type_t::action;
		if(strcmp(type, "mod") == 0)
		{
			macro.type = keymacro_t::type_t::mod;
			macro.mod.persist = false;
		}
		if(strcmp(type, "hid") == 0)
		{
			macro.type = keymacro_t::type_t::hid_key;
			macro.hid_key.modifier = 0;
			macro.hid_key.keycode = HID_KEY_ENTER;
		}

		if(macro.type == keymacro_t::type_t::hid_key)
		{
			const char *value = json_getPropertyValue(entry, "v");
			const char *modifier = json_getPropertyValue(entry, "m");

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
			{
#define CONCAT_H(x,y,z) x##y##z
#define SINGLEQUOTE '
#define CONCAT(x,y,z) CONCAT_H(x,y,z)
#define CHARIFY(x) CONCAT(SINGLEQUOTE, x , SINGLEQUOTE)

				if(value[1] == '\0')
				{
					switch(value[0])
					{
#define MAP_SINGLE_CHAR(key) \
						case CHARIFY(key): \
							macro.hid_key.keycode = HID_KEY_##key; \
							break

						MAP_SINGLE_CHAR(A);
						MAP_SINGLE_CHAR(B);
						MAP_SINGLE_CHAR(C);
						MAP_SINGLE_CHAR(D);
						MAP_SINGLE_CHAR(E);
						MAP_SINGLE_CHAR(F);
						MAP_SINGLE_CHAR(G);
						MAP_SINGLE_CHAR(H);
						MAP_SINGLE_CHAR(I);
						MAP_SINGLE_CHAR(J);
						MAP_SINGLE_CHAR(K);
						MAP_SINGLE_CHAR(L);
						MAP_SINGLE_CHAR(M);
						MAP_SINGLE_CHAR(N);
						MAP_SINGLE_CHAR(O);
						MAP_SINGLE_CHAR(P);
						MAP_SINGLE_CHAR(Q);
						MAP_SINGLE_CHAR(R);
						MAP_SINGLE_CHAR(S);
						MAP_SINGLE_CHAR(T);
						MAP_SINGLE_CHAR(U);
						MAP_SINGLE_CHAR(V);
						MAP_SINGLE_CHAR(W);
						MAP_SINGLE_CHAR(X);
						MAP_SINGLE_CHAR(Y);
						MAP_SINGLE_CHAR(Z);

						MAP_SINGLE_CHAR(0);
						MAP_SINGLE_CHAR(1);
						MAP_SINGLE_CHAR(2);
						MAP_SINGLE_CHAR(3);
						MAP_SINGLE_CHAR(4);
						MAP_SINGLE_CHAR(5);
						MAP_SINGLE_CHAR(6);
						MAP_SINGLE_CHAR(7);
						MAP_SINGLE_CHAR(8);
						MAP_SINGLE_CHAR(9);

						case ' ':
							macro.hid_key.keycode = HID_KEY_SPACE;
							break;

						default:
							break;
#undef MAP_SINGLE_CHAR
					}
				}
				else
				{
					if(strcmp(value, "enter") == 0)
						macro.hid_key.keycode = HID_KEY_ENTER;
					if(strcmp(value, "ESC") == 0)
						macro.hid_key.keycode = HID_KEY_ESCAPE;
					if(strcmp(value, "DEL") == 0)
						macro.hid_key.keycode = HID_KEY_DELETE;
					if(strcmp(value, "TAB") == 0)
						macro.hid_key.keycode = HID_KEY_TAB;
				}
			}
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
	result->title = json_getPropertyValue(keymap, "name");
	result->active_page = 0;

	if(!result->title)
		result->title = "No name";

	for(const json_t *layer = json_getChild(layers); layer; layer = json_getSibling(layer))
	{
		const json_t *base = json_getProperty(layer, "base");
		if(!base)
			continue;

		keylayer_t parsed;
		parse_macros(parsed.macros, base);

		const json_t *mod = json_getProperty(layer, "mod");
		if(mod)
			parse_macros(parsed.mod_macros, base);

		result->layers.push_back(parsed);
	}

	if(result->layers.empty())
	{
		delete result;
		return nullptr;
	}

	return result;
}
