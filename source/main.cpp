//
// Created by Sidney on 03/07/2025.
//

#include <config.h>
#include <bsp/board_api.h>
#include "usb/usb_descriptor.h"
#include "logic/application.h"
#include "logic/flashfs.h"

static application app;

[[noreturn]] int main()
{
	board_init();

	flashfs_init();

	tud_init(BOARD_DEVICE_RHPORT_NUM);
	board_init_after_tusb();

	app.init();

	while(true)
	{
		app.update();
		sleep_ms(5);
	}
}

void tud_mount_cb()
{
	app.usb_state_changed();
}
void tud_umount_cb()
{
	app.usb_state_changed();
}

void tud_suspend_cb(bool remote_wakeup_en)
{
	app.usb_state_changed();
}
void tud_resume_cb()
{
	app.usb_state_changed();
}

void usb_ejected()
{
	flashfs_flush();
	app.load_configuration();
}
