//
// Created by Sidney on 22/08/2025.
//

#include "usb_descriptor.h"
#include "../logic/flashfs.h"

extern void usb_ejected();

void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
	const char vid[] = "TinyUSB";
	const char pid[] = "Mass Storage";
	const char rev[] = "1.0";

	memcpy(vendor_id, vid, strlen(vid));
	memcpy(product_id, pid, strlen(pid));
	memcpy(product_rev, rev, strlen(rev));
}

bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
	return true;
}

void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size)
{
	*block_count = DISK_SECTOR_COUNT;
	*block_size = DISK_SECTOR_SIZE;
}


bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
	if(load_eject && !start)
		usb_ejected();

	return true;
}

int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
	if(lba >= DISK_SECTOR_COUNT || offset != 0 || bufsize != DISK_SECTOR_SIZE)
		return - 1;

	flashfs_read(buffer, lba, offset, bufsize);
	return (int32_t) bufsize;
}

bool tud_msc_is_writable_cb(uint8_t lun)
{
	return true;
}

int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
	if(lba >= DISK_SECTOR_COUNT || offset != 0 || bufsize != DISK_SECTOR_SIZE)
		return - 1;

	flashfs_write(buffer, lba, offset, bufsize);
	return (int32_t) bufsize;
}

int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer, uint16_t bufsize)
{
	tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);
	return -1;
}
