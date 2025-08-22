//
// Created by Sidney on 04/07/2025.
//

#include <hardware/gpio.h>
#include <pico/time.h>
#include "keymatrix.h"

void keymatrix_t::init(const std::span<const uint32_t> &row_pins, const std::span<const uint32_t> &column_pins)
{
	m_rows = row_pins;
	m_columns = column_pins;

	for(auto pin : m_rows)
	{
		gpio_init(pin);
		gpio_set_dir(pin, GPIO_OUT);
	}

	for(auto pin : m_columns)
	{
		gpio_init(pin);
		gpio_set_dir(pin, GPIO_IN);
		gpio_pull_down(pin);
	}

	update();
	m_changes = 0;
}

void keymatrix_t::update()
{
	m_changes = 0;

	for(size_t i = 0; i < m_rows.size(); i ++)
	{
		gpio_put(m_rows[i], true);

		for(uint j = 0; j < m_columns.size(); j ++)
		{
			const uint32_t index = index_for_coord(i, j);

			const bool previous = m_state & (1 << index);
			bool result = gpio_get(m_columns[j]);

			if(result != previous)
			{
				sleep_ms(1);
				result = gpio_get(m_columns[j]);
			}

			if(result != previous)
			{
				if(result)
					m_state |= (1 << index);
				else
					m_state &= ~(1 << index);

				m_changes |= 1 << index;
			}
		}

		gpio_put(m_rows[i], false);
		sleep_ms(1);
	}
}
