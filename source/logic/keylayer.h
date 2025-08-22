//
// Created by Sidney on 04/07/2025.
//

#ifndef KEYLAYER_H
#define KEYLAYER_H

#include <cstdint>
#include <vector>
#include <config.h>

enum class action_t
{
	flash,
	brightness_up,
	brightness_down,
};

struct keymacro_t
{
	enum class type_t
	{
		none,
		hid_key,
		action,
		mod,
	};

	type_t type = type_t::none;

	union
	{
		struct
		{
			uint8_t modifier;
			uint8_t keycode;
		} hid_key;

		struct
		{
			action_t action;
		} action;

		struct
		{
			bool persist;
		} mod;
	};
};

struct keylayer_t
{
	keymacro_t macros[num_key_rows][num_key_cols] = {};
	keymacro_t mod_macros[num_key_rows][num_key_cols] = {};
};

struct keymap_t
{
	const char *title;
	std::vector<keylayer_t> layers;
	uint8_t active_page;
};

extern keymap_t *build_blender_keymap();
extern keymap_t *build_fusion_keymap();
extern keymap_t *build_system_keymap();

#endif //KEYLAYER_H
