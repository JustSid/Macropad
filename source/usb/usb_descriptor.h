//
// Created by Sidney on 04/07/2025.
//

#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include <tusb.h>

enum
{
	REPORT_ID_KEYBOARD = 1,
	REPORT_ID_COUNT
};

enum
{
	STR_ID_LANGID = 0,
	STR_ID_MANUFACTURER,
	STR_ID_PRODUCT,
	STR_ID_SERIAL,
};

#define USB_FEATURE_HID (1 << 0)
#define USB_FEATURE_MSC (1 << 1)

void usb_set_enabled_features(uint8_t flags);

#endif
