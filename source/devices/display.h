//
// Created by Sidney on 03/07/2025.
//

#ifndef DISPLAY_H
#define DISPLAY_H

#include <hardware/i2c.h>

class display_t
{
public:
	display_t() = default;
	~display_t();

	bool init(i2c_inst_t *i2c, uint16_t width, uint16_t height, uint32_t address);
	bool update();

	uint8_t get_contrast() const { return m_contrast; }
	bool set_contrast(uint8_t contrast);

	void clear();
	void set_pixel(uint16_t x, uint16_t y, bool on);
	void toggle_pixel(uint16_t x, uint16_t y);

	void fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool on);
	void stroke_line_horizontal(uint16_t x, uint16_t y, uint16_t length, bool on);

private:
	uint32_t get_num_bytes() const { return m_width * ((m_height + 7) / 8); }

	bool send_command(uint8_t command, bool nostop) const;
	bool send_command_list(uint8_t base, const uint8_t *data, size_t count, bool nostop) const;

	i2c_inst_t *m_i2c = nullptr;
	uint32_t m_address = 0;
	uint8_t *m_buffer = nullptr;

	uint16_t m_width = 0;
	uint16_t m_height = 0;

	uint8_t m_contrast = 255;
};

#endif //DISPLAY_H
