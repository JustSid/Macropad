//
// Created by Sidney on 04/07/2025.
//

#ifndef ANALOGSTICK_H
#define ANALOGSTICK_H

#include <cstdint>
#include <hardware/adc.h>

class analogstick_t
{
public:
	analogstick_t() = default;

	void init(uint32_t x_gpio, uint32_t y_gpio);
	void update();

	uint16_t get_x_value(uint16_t scalar) const { return m_x_value / (4096 / scalar); }
	uint16_t get_y_value(uint16_t scalar) const { return m_y_value / (4096 / scalar); }

private:
	uint32_t m_x_gpio = 0;
	uint32_t m_y_gpio = 0;

	uint16_t m_x_value = 0;
	uint16_t m_y_value = 0;
};

#endif //ANALOGSTICK_H
