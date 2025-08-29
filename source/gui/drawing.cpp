//
// Created by Sidney on 04/07/2025.
//

#include <cstring>
#include <algorithm>
#include <config.h>
#include "font.h"
#include "drawing.h"

uint16_t draw_string(display_t *display, const char *text, bool foreground, uint16_t start_x, uint16_t start_y, uint16_t max_width, uint16_t max_lines)
{
	const size_t length = strlen(text);

	uint16_t offset_x = 0;
	uint16_t offset_y = 0;

	uint16_t greatest_width = 0;

	for(size_t i = 0; i < length; i ++)
	{
		const uint8_t *font_data = g_font[text[i]];

		for(uint16_t y = 0; y < font_height; y ++)
		{
			const uint8_t slice = font_data[y];

			for(uint16_t x = 0; x < font_width; x ++)
			{
				if(slice & (1 << x))
					display->set_pixel(start_x + offset_x + (font_width - x), start_y + offset_y + y, foreground);
			}
		}

		offset_x += font_width;

		if(offset_x >= max_width)
		{
			greatest_width = std::max(greatest_width, offset_x);

			offset_x = 0;
			offset_y += font_height;
		}
	}

	return std::max(greatest_width, offset_x);
}

uint16_t draw_string(display_t *display, const char *text, bool foreground, uint16_t start_x, uint16_t start_y, uint16_t max_width, text_justification_t justification)
{
	const uint16_t max_chars = max_width / font_width;

	switch(justification)
	{
		case text_justification_t::left:
			return draw_string(display, text, foreground, start_x, start_y, max_width);

		case text_justification_t::center:
		{
			const size_t length = std::min(strlen(text), size_t(max_chars));
			const uint16_t offset = ((max_chars - length) / 2) * font_width;

			return draw_string(display, text, foreground, start_x + offset, start_y, length * font_width, 1);
		}

		case text_justification_t::right:
		{
			const size_t length = std::min(strlen(text), size_t(max_chars));
			const uint16_t offset = (max_chars - length) * font_width;

			return draw_string(display, text, foreground, start_x + offset, start_y, length * font_width, 1);
		}
	}

	return 0;
}