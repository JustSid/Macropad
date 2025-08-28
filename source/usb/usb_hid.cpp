//
// Created by Sidney on 28/08/2025.
//

#include "usb_descriptor.h"

static uint8_t desc_hid_report[] =
{
	TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_ID_KEYBOARD)),
};

size_t usb_get_hid_report_desc_len()
{
	return sizeof(desc_hid_report);
}

const uint8_t *tud_hid_descriptor_report_cb(uint8_t instance)
{
	return desc_hid_report;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
	return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{}
