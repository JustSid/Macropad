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

#endif
