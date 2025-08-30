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
	const uint32_t now = to_ms_since_boot(get_absolute_time());
	if((now - m_last_update) < 10)
		return;

	m_last_update = now;
	m_changes = 0;

	for(size_t i = 0; i < m_rows.size(); i ++)
	{
		gpio_put(m_rows[i], true);
		sleep_us(10);

		for(uint j = 0; j < m_columns.size(); j ++)
		{
			const uint32_t index = index_for_coord(i, j);

			const bool previous = m_state & (1 << index);
			const bool result = gpio_get(m_columns[j]);

			const uint32_t bit = (1 << index);

			if(result != previous)
			{
				if(m_pending & bit)
				{
					if(result)
						m_state |= bit;
					else
						m_state &= ~bit;

					m_changes |= bit;
				}
				else
					m_pending |= bit;
			}
			else
				m_pending &= ~bit;
		}

		gpio_put(m_rows[i], false);
		sleep_us(50);
	}
}
