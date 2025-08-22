//
// Created by Sidney on 04/07/2025.
//

#include <iterator>
#include <bsp/board_api.h>
#include "usb_descriptor.h"

//--------------------------------------------------------------------+
// Device Descriptor
//--------------------------------------------------------------------+

#define PID_MAP(itf, n)   ((CFG_TUD_##itf) << (n))
#define USB_PID           (0x4000 | PID_MAP(CDC, 0) | PID_MAP(MSC, 1) | PID_MAP(HID, 2) | PID_MAP(MIDI, 3) | PID_MAP(VENDOR, 4))

#define USB_VID 0xcafe
#define USB_BCD 0x0200

static tusb_desc_device_t s_desc_device =
{
	.bLength = sizeof(tusb_desc_device_t),
	.bDescriptorType = TUSB_DESC_DEVICE,
	.bcdUSB = USB_BCD,
	.bDeviceClass = 0x00,
	.bDeviceSubClass = 0x00,
	.bDeviceProtocol = 0x00,
	.bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

	.idVendor = USB_VID,
	.idProduct = USB_PID,
	.bcdDevice = 0x0100,

	.iManufacturer = STR_ID_MANUFACTURER,
	.iProduct = STR_ID_PRODUCT,
	.iSerialNumber = STR_ID_SERIAL,

	.bNumConfigurations = 0x01
};

const uint8_t *tud_descriptor_device_cb()
{
	return (const uint8_t *)&s_desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

static uint8_t desc_hid_report[] =
{
	TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_ID_KEYBOARD)),
};

const uint8_t *tud_hid_descriptor_report_cb(uint8_t instance)
{
	return desc_hid_report;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum
{
	ITF_NUM_HID,
	ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
#define EPNUM_HID   0x81

const uint8_t desc_configuration[] =
{
	// Config number, interface count, string index, total length, attribute, power in mA
	TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

	// Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
	TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 5)
};

const uint8_t *tud_descriptor_configuration_cb(uint8_t index)
{
	return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

const char *string_desc_arr[] =
{
	(const char[]){0x09, 0x04}, // 0: Supported language (English, 0x0409)
	"Sidney Just",              // 1: Manufacturer
	"Macropad",                 // 2: Product
	nullptr,                    // 3: Serials will use unique board ID
};

static uint16_t desc_str_temp[32 + 1];

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
	if(index >= std::size(string_desc_arr))
		return nullptr;

	size_t count;

	switch(index)
	{
		case STR_ID_LANGID:
			memcpy(&desc_str_temp[1], string_desc_arr[0], 2);
			count = 1;
			break;
		case STR_ID_SERIAL:
			count = board_usb_get_serial(desc_str_temp + 1, 32);
			break;

		default:
			const char *str = string_desc_arr[index];
			count = std::min(strlen(str), std::size(desc_str_temp) - 1);

			for(size_t i = 0; i < count; i++)
				desc_str_temp[1 + i] = str[i];

			break;
	}

	desc_str_temp[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * count + 2));
	return desc_str_temp;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
	return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{}
