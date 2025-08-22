//
// Created by Sidney on 04/07/2025.
//

#include "analogstick.h"

void analogstick_t::init(uint32_t x_gpio, uint32_t y_gpio)
{
	m_x_value = x_gpio;
	m_y_gpio = y_gpio;

	adc_init();
	adc_gpio_init(m_x_gpio);
	adc_gpio_init(m_y_gpio);
}

void analogstick_t::update()
{
	adc_select_input(0);
	m_x_value = adc_read();

	adc_select_input(1);
	m_y_value = adc_read();
}