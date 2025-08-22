//
// Created by Sidney on 03/07/2025.
//

#include <cstring>
#include <algorithm>
#include "display.h"

#define SSD1306_MEMORYMODE 0x20          ///< See datasheet
#define SSD1306_COLUMNADDR 0x21          ///< See datasheet
#define SSD1306_PAGEADDR 0x22            ///< See datasheet
#define SSD1306_SETCONTRAST 0x81         ///< See datasheet
#define SSD1306_CHARGEPUMP 0x8D          ///< See datasheet
#define SSD1306_SEGREMAP 0xA0            ///< See datasheet
#define SSD1306_DISPLAYALLON_RESUME 0xA4 ///< See datasheet
#define SSD1306_DISPLAYALLON 0xA5        ///< Not currently used
#define SSD1306_NORMALDISPLAY 0xA6       ///< See datasheet
#define SSD1306_INVERTDISPLAY 0xA7       ///< See datasheet
#define SSD1306_SETMULTIPLEX 0xA8        ///< See datasheet
#define SSD1306_DISPLAYOFF 0xAE          ///< See datasheet
#define SSD1306_DISPLAYON 0xAF           ///< See datasheet
#define SSD1306_COMSCANINC 0xC0          ///< Not currently used
#define SSD1306_COMSCANDEC 0xC8          ///< See datasheet
#define SSD1306_SETDISPLAYOFFSET 0xD3    ///< See datasheet
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5  ///< See datasheet
#define SSD1306_SETPRECHARGE 0xD9        ///< See datasheet
#define SSD1306_SETCOMPINS 0xDA          ///< See datasheet
#define SSD1306_SETVCOMDETECT 0xDB       ///< See datasheet

#define SSD1306_SETLOWCOLUMN 0x00  ///< Not currently used
#define SSD1306_SETHIGHCOLUMN 0x10 ///< Not currently used
#define SSD1306_SETSTARTLINE 0x40  ///< See datasheet

#define SSD1306_DEACTIVATE_SCROLL 0x2E                    ///< Stop scroll

display_t::~display_t()
{
	delete[] m_buffer;
}


bool display_t::send_command(uint8_t command, bool nostop) const
{
	uint8_t data[] = { 0x0, command };
	return i2c_write_blocking(m_i2c, m_address, data, 2, nostop) >= 0;
}
bool display_t::send_command_list(uint8_t base, const uint8_t *data, size_t count, bool nostop) const
{
	uint8_t bytes[128] = { base };
	uint16_t written = 1;

	while(count --)
	{
		if(written >= 128)
		{
			const int result = i2c_write_blocking(m_i2c, m_address, bytes, written, true);

			if(result < 0)
				return false;

			bytes[0] = base;
			written = 1;
		}

		bytes[written ++] = *data ++;
	}

	if(written > 1)
		return i2c_write_blocking(m_i2c, m_address, bytes, written, nostop) >= 0;

	return true;
}

bool display_t::init(i2c_inst_t *i2c, uint16_t width, uint16_t height, uint32_t address)
{
	m_i2c = i2c;
	m_width = width;
	m_height = height;
	m_address = address;

	const size_t count = get_num_bytes();

	m_buffer = new uint8_t[count];
	memset(m_buffer, 0, count);


	const uint8_t data1[] = { SSD1306_DISPLAYOFF, SSD1306_SETDISPLAYCLOCKDIV, 0x80, SSD1306_SETMULTIPLEX };
	if(!send_command_list(0x0, data1, sizeof(data1), true))
		return false;

	if(!send_command(m_height - 1, true))
		return false;

	const uint8_t data2[] = { SSD1306_SETDISPLAYOFFSET, 0x0, SSD1306_SETSTARTLINE | 0x0, SSD1306_CHARGEPUMP };
	if(!send_command_list(0x0, data2, sizeof(data2), true))
		return false;

	bool external_vcc = false;

	if(!send_command(external_vcc ? 0x10 : 0x14, true))
		return false;

	const uint8_t data3[] = { SSD1306_MEMORYMODE, 0x00, SSD1306_SEGREMAP | 0x1, SSD1306_COMSCANDEC };
	if(!send_command_list(0x0, data3, sizeof(data3), true))
		return false;

	if(!send_command(SSD1306_SETCOMPINS, true))
		return false;
	if(!send_command(0x02, true))
		return false;

	if(!send_command(SSD1306_SETCONTRAST, true))
		return false;
	if(!send_command(m_contrast, true))
		return false;

	if(!send_command(SSD1306_SETPRECHARGE, true))
		return false;
	if(!send_command(external_vcc ? 0x22 : 0xF1, true))
		return false;

	const uint8_t data4[] = { SSD1306_SETVCOMDETECT, 0x40, SSD1306_DISPLAYALLON_RESUME, SSD1306_NORMALDISPLAY, SSD1306_DEACTIVATE_SCROLL, SSD1306_DISPLAYON };
	if(!send_command_list(0x0, data4, sizeof(data4), false))
		return false;

	return true;
}

bool display_t::set_contrast(uint8_t contrast)
{
	if(contrast == m_contrast)
		return true;

	if(!send_command(SSD1306_SETCONTRAST, true))
		return false;
	if(!send_command(contrast, false))
		return false;

	m_contrast = contrast;

	return true;
}

bool display_t::update()
{
	const uint8_t data1[] = { SSD1306_PAGEADDR, 0x0, 0xff, SSD1306_COLUMNADDR };
	if(!send_command_list(0x0, data1, sizeof(data1), true))
		return false;

	if(!send_command(0x0, true))
		return false;
	if(!send_command(m_width - 1, true))
		return false;

	const uint16_t count = get_num_bytes();
	return send_command_list(0x40, m_buffer, count, false);
}



void display_t::clear()
{
	const uint16_t bytes = get_num_bytes();
	memset(m_buffer, 0, bytes);
}

void display_t::set_pixel(uint16_t x, uint16_t y, bool on)
{
	if(x >= m_width || y >= m_height)
		return;

	const uint32_t index = x + (y / 8) * m_width;
	const uint8_t bit = (1 << (y & 7));

	if(on)
		m_buffer[index] |= bit;
	else
		m_buffer[index] &= ~bit;
}

void display_t::toggle_pixel(uint16_t x, uint16_t y)
{
	if(x >= m_width || y >= m_height)
		return;

	const uint32_t index = x + (y / 8) * m_width;
	const uint8_t bit = (1 << (y & 7));

	m_buffer[index] ^= bit;
}

void display_t::fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool on)
{
	if(x >= m_width || y >= m_height)
		return;

	h = std::min(m_height, uint16_t(y + h)) - y;
	w = std::min(m_width, uint16_t(x + w)) - x;

	while(y & 7 && h > 0)
	{
		const uint8_t bit = (1 << (y & 7));

		uint32_t index = x + (y / 8) * m_width;

		for(uint32_t i = 0; i < w; i ++)
		{
			if(on)
				m_buffer[index ++] |= bit;
			else
				m_buffer[index ++] &= ~bit;
		}

		y ++;
		h --;
	}

	while(h >= 8)
	{
		uint32_t index = x + (y / 8) * m_width;

		for(uint32_t i = 0; i < w; i ++)
		{
			if(on)
				m_buffer[index ++] = 0xff;
			else
				m_buffer[index ++] = 0x0;
		}

		y += 8;
		h -= 8;
	}

	while(h > 0)
	{
		const uint8_t bit = (1 << (y & 7));

		uint32_t index = x + (y / 8) * m_width;

		for(uint32_t i = 0; i < w; i ++)
		{
			if(on)
				m_buffer[index ++] |= bit;
			else
				m_buffer[index ++] &= ~bit;
		}

		y ++;
		h --;
	}
}

void display_t::stroke_line_horizontal(uint16_t x, uint16_t y, uint16_t length, bool on)
{
	if(x >= m_width)
		return;

	length = std::min(m_width, uint16_t(x + length)) - x;

	const uint8_t bit = (1 << (y & 7));
	uint32_t index = x + (y / 8) * m_width;

	for(uint32_t i = 0; i < length; i ++)
	{
		if(on)
			m_buffer[index ++] |= bit;
		else
			m_buffer[index ++] &= ~bit;
	}
}
