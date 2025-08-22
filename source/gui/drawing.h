//
// Created by Sidney on 04/07/2025.
//

#ifndef DRAWING_H
#define DRAWING_H

#include <devices/display.h>

enum class text_justification_t
{
	left,
	center,
	right
};

void draw_string(display_t *display, const char *text, bool foreground, uint16_t start_x, uint16_t start_y, uint16_t max_width, uint16_t max_lines = 1);
void draw_string(display_t *display, const char *text, bool foreground, uint16_t start_x, uint16_t start_y, uint16_t max_width, text_justification_t justification);

#endif //DRAWING_H
