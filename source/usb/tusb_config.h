#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

// defined by board.mk
#ifndef CFG_TUSB_MCU
	#error CFG_TUSB_MCU must be defined
#endif

#ifndef CFG_TUSB_OS
	#error CFG_TUSB_OS must be defined
#endif

// RHPort number used for device can be defined by board.mk, default to port 0
#ifndef BOARD_DEVICE_RHPORT_NUM
	#define BOARD_DEVICE_RHPORT_NUM     0
#endif

#define BOARD_DEVICE_RHPORT_SPEED   OPT_MODE_FULL_SPEED

// Device mode with rhport and speed defined by board.mk
#if BOARD_DEVICE_RHPORT_NUM == 0
	#define CFG_TUSB_RHPORT0_MODE     (OPT_MODE_DEVICE | BOARD_DEVICE_RHPORT_SPEED)
#elif BOARD_DEVICE_RHPORT_NUM == 1
	#define CFG_TUSB_RHPORT1_MODE     (OPT_MODE_DEVICE | BOARD_DEVICE_RHPORT_SPEED)
#else
	#error "Incorrect RHPort configuration"
#endif

#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN          __attribute__ ((aligned(4)))
#endif

#define CFG_TUD_ENABLED       1

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE    64
#endif

//------------- CLASS -------------//
#define CFG_TUD_HID               1
#define CFG_TUD_CDC               0
#define CFG_TUD_MSC               1
#define CFG_TUD_MIDI              0
#define CFG_TUD_VENDOR            0

// MSC Buffer size of Device Mass storage
#define CFG_TUD_MSC_EP_BUFSIZE    512

// HID buffer size Should be sufficient to hold ID (if any) + Data
#define CFG_TUD_HID_EP_BUFSIZE    16

#endif /* _TUSB_CONFIG_H_ */
