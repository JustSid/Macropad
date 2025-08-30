//
// Created by Sidney on 04/07/2025.
//

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <cstdint>
#include <span>

class keymatrix_t
{
public:
	keymatrix_t() = default;

	void init(const std::span<const uint32_t> &row_pins, const std::span<const uint32_t> &column_pins);
	void update();

	bool get_state(uint32_t row, uint32_t column) const { return m_state & (1 << index_for_coord(row, column)); }

	bool has_state_changed() const { return m_changes > 0; }
	bool has_state_changed(uint32_t row, uint32_t column) const { return m_changes & (1 << index_for_coord(row, column)); }
	bool has_changed_to_enabled(uint32_t row, uint32_t column) const { const uint32_t test = 1 << index_for_coord(row, column); return m_changes & test && m_state & test; }

	bool has_any_events() const { return m_state != 0 || m_changes != 0; }

private:
	uint32_t index_for_coord(uint32_t row, uint32_t column) const { return row * m_columns.size() + column; };

	uint32_t m_state = 0;
	uint32_t m_changes = 0;
	uint32_t m_pending = 0;

	uint32_t m_last_update = 0;

	std::span<const uint32_t> m_rows;
	std::span<const uint32_t> m_columns;
};

#endif //KEYBOARD_H
