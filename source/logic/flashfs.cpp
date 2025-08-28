//
// Created by Sidney on 22/08/2025.
//

#include <ff.h>
#include <diskio.h>
#include <cstring>
#include <hardware/flash.h>
#include <hardware/sync.h>
#include <bsp/board_api.h>
#include "flashfs.h"

static_assert(DISK_SECTOR_SIZE <= FF_MAX_SS);

typedef struct __attribute__((packed))
{
	uint8_t  jump_boot[3];           // 0x00: Jump instruction
	char     oem_name[8];            // 0x03: OEM name
	uint16_t bytes_per_sector;       // 0x0B: Bytes per sector
	uint8_t  sectors_per_cluster;    // 0x0D: Sectors per cluster
	uint16_t reserved_sectors;       // 0x0E: Reserved sectors
	uint8_t  num_fats;              // 0x10: Number of FATs
	uint16_t root_entries;          // 0x11: Root directory entries (0 for FAT32)
	uint16_t total_sectors_16;      // 0x13: Total sectors (0 if > 65535)
	uint8_t  media_descriptor;      // 0x15: Media descriptor
	uint16_t fat_size_16;           // 0x16: FAT size in sectors (0 for FAT32)
	uint16_t sectors_per_track;     // 0x18: Sectors per track
	uint16_t num_heads;             // 0x1A: Number of heads
	uint32_t hidden_sectors;        // 0x1C: Hidden sectors
	uint32_t total_sectors_32;      // 0x20: Total sectors (if > 65535)

	// FAT32 extended fields
	uint32_t fat_size_32;           // 0x24: FAT size in sectors
	uint16_t ext_flags;             // 0x28: Extended flags
	uint16_t fs_version;            // 0x2A: Filesystem version
	uint32_t root_cluster;          // 0x2C: Root directory cluster
	uint16_t fs_info_sector;        // 0x30: FSInfo sector number
	uint16_t backup_boot_sector;    // 0x32: Backup boot sector
	uint8_t  reserved1[12];         // 0x34: Reserved
	uint8_t  drive_number;          // 0x40: Drive number
	uint8_t  reserved2;             // 0x41: Reserved
	uint8_t  boot_signature;        // 0x42: Boot signature (0x29)
	uint32_t volume_id;             // 0x43: Volume ID
	char     volume_label[11];      // 0x47: Volume label
	char     fs_type[8];            // 0x52: Filesystem type
	uint8_t  boot_code[420];        // 0x5A: Boot code
	uint16_t boot_sector_signature; // 0x1FE: Boot sector signature (0xAA55)
} fat32_boot_sector_t;

typedef struct __attribute__((packed))
{
	uint32_t lead_signature;        // 0x00: Lead signature (0x41615252)
	uint8_t  reserved1[480];        // 0x04: Reserved
	uint32_t struct_signature;      // 0x1E4: Structure signature (0x61417272)
	uint32_t free_count;            // 0x1E8: Free cluster count
	uint32_t next_free;             // 0x1EC: Next free cluster
	uint8_t  reserved2[12];         // 0x1F0: Reserved
	uint32_t trail_signature;       // 0x1FC: Trail signature (0xAA550000)
} fat32_fsinfo_t;

void create_fat32_header(void *buffer, uint32_t total_sectors, uint32_t bytes_per_sector)
{
	// Choose sectors per cluster (power of 2, typically 1-128)
	// For simplicity, use 1 sector per cluster for small drives
	uint8_t sectors_per_cluster = (total_sectors < 65536) ? 1 : 8;

	// Calculate FAT size
	uint32_t reserved_sectors = 32;  // Standard for FAT32
	uint32_t num_fats = 2;           // Standard number of FATs
	uint32_t root_cluster = 2;       // Root directory starts at cluster 2

	// Estimate clusters and FAT size
	uint32_t data_sectors = total_sectors - reserved_sectors;
	uint32_t clusters = data_sectors / sectors_per_cluster;
	uint32_t fat_size = (clusters * 4 + bytes_per_sector - 1) / bytes_per_sector;

	// Adjust for FAT space
	data_sectors = total_sectors - reserved_sectors - (num_fats * fat_size);
	clusters = data_sectors / sectors_per_cluster;

	// Initialize boot sector
	fat32_boot_sector_t *boot = (fat32_boot_sector_t *)buffer;
	memset(boot, 0, sizeof(fat32_boot_sector_t));

	// Jump instruction (typical x86 jump)
	boot->jump_boot[0] = 0xEB;
	boot->jump_boot[1] = 0xE9;
	boot->jump_boot[2] = 0xE8;

	// OEM name
	memcpy(boot->oem_name, "MSWIN4.1", 8);

	// BPB (BIOS Parameter Block)
	boot->bytes_per_sector = bytes_per_sector;
	boot->sectors_per_cluster = sectors_per_cluster;
	boot->reserved_sectors = reserved_sectors;
	boot->num_fats = num_fats;
	boot->root_entries = 0;  // 0 for FAT32
	boot->total_sectors_16 = (total_sectors < 65536) ? total_sectors : 0;
	boot->media_descriptor = 0xF8;  // Fixed disk
	boot->fat_size_16 = 0;  // 0 for FAT32
	boot->sectors_per_track = 63;
	boot->num_heads = 255;
	boot->hidden_sectors = 0;
	boot->total_sectors_32 = (total_sectors >= 65536) ? total_sectors : 0;

	// FAT32 extended fields
	boot->fat_size_32 = fat_size;
	boot->ext_flags = 0;
	boot->fs_version = 0;
	boot->root_cluster = root_cluster;
	boot->fs_info_sector = 1;
	boot->backup_boot_sector = 6;
	boot->drive_number = 0x80;  // Hard disk
	boot->boot_signature = 0x29;
	boot->volume_id = 0x12345678;  // Arbitrary volume ID
	memcpy(boot->volume_label, "NO NAME    ", 11);
	memcpy(boot->fs_type, "FAT32   ", 8);
	boot->boot_sector_signature = 0xAA55;

	// Create FSInfo sector
	fat32_fsinfo_t *fsinfo = (fat32_fsinfo_t *)((uint8_t *)buffer + bytes_per_sector);
	memset(fsinfo, 0, sizeof(fat32_fsinfo_t));
	fsinfo->lead_signature = 0x41615252;
	fsinfo->struct_signature = 0x61417272;
	fsinfo->free_count = clusters - 1;  // All clusters except root
	fsinfo->next_free = 3;  // First available cluster after root
	fsinfo->trail_signature = 0xAA550000;

	// Initialize first FAT
	uint32_t *fat = (uint32_t *)((uint8_t *)buffer + reserved_sectors * bytes_per_sector);
	fat[0] = 0x0FFFFFF8;  // Media descriptor in cluster 0
	fat[1] = 0x0FFFFFFF;  // End of chain marker in cluster 1
	fat[2] = 0x0FFFFFFF;  // Root directory end of chain

	// Initialize second FAT (copy of first)
	uint32_t *fat2 = (uint32_t *)((uint8_t *)fat + fat_size * bytes_per_sector);
	memcpy(fat2, fat, fat_size * bytes_per_sector);

	// Clear root directory area
	uint8_t *root_dir = (uint8_t *)buffer + (reserved_sectors + num_fats * fat_size + (root_cluster - 2) * sectors_per_cluster) * bytes_per_sector;
	memset(root_dir, 0, sectors_per_cluster * bytes_per_sector);
}

struct ram_flash_disk_t
{
	uint8_t version;
	size_t size;

	uint8_t data[DISK_SECTOR_COUNT][DISK_SECTOR_SIZE];
};

static ram_flash_disk_t ram_disk;
static FATFS fs;

#define FLASHFS_VERSION 1
#define FLASH_SECTOR_COUNT 17

static_assert(FLASH_SECTOR_SIZE * FLASH_SECTOR_COUNT >= sizeof(ram_flash_disk_t));
static_assert(PICO_FLASH_SIZE_BYTES >= FLASH_SECTOR_SIZE * FLASH_SECTOR_COUNT);

#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - (FLASH_SECTOR_SIZE * FLASH_SECTOR_COUNT))

bool flashfs_read_disk(ram_flash_disk_t *target)
{
	const uint8_t *flash = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
	memcpy(target, flash, sizeof(ram_flash_disk_t));

	if(target->version != FLASHFS_VERSION || target->size != sizeof(ram_flash_disk_t))
	{
		target->version = FLASHFS_VERSION;
		target->size = sizeof(ram_flash_disk_t);

		memset(target->data, 0, DISK_SECTOR_COUNT * DISK_SECTOR_SIZE);
		create_fat32_header(target->data, DISK_SECTOR_COUNT, DISK_SECTOR_SIZE);
		return false;
	}

	return true;
}

void flashfs_create_initial_files()
{
	FIL file;

	if(f_open(&file, "/readme.txt", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
	{
		UINT bw;
		const char *readme = "Use the config.json file to store your keymap configuration.\n"
							 "Take a look at the repo contents for examples!\n";

		f_write(&file, readme, strlen(readme), &bw);
		f_close(&file);
	}

	if(f_open(&file, "/config.json", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
	{
		UINT bw;
		const char *json = "[\n]\n";
		f_write(&file, json, strlen(json), &bw);
		f_close(&file);
	}
}

void flashfs_init()
{
	const bool has_disk = flashfs_read_disk(&ram_disk);

	FRESULT res = f_mount(&fs, "/", 1);
	assert(res == FR_OK);

	if(!has_disk)
		flashfs_create_initial_files();
}
void flashfs_read(void *buffer, uint32_t lba, uint32_t offset, uint32_t length)
{
	uint8_t *addr = ram_disk.data[lba] + offset;
	memcpy(buffer, addr, length);
}
void flashfs_write(void *buffer, uint32_t lba, uint32_t offset, uint32_t length)
{
	uint8_t *addr = ram_disk.data[lba] + offset;
	memcpy(addr, buffer, length);
}
void flashfs_flush()
{
	const uint32_t ints = save_and_disable_interrupts();

	board_led_on();
	flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE * FLASH_SECTOR_COUNT);

	size_t remaining = sizeof(ram_flash_disk_t);

	const uint8_t *ptr = (const uint8_t *)&ram_disk;
	size_t offset = 0;

	while(remaining >= FLASH_PAGE_SIZE)
	{
		flash_range_program(FLASH_TARGET_OFFSET + offset, ptr, FLASH_PAGE_SIZE);

		ptr += FLASH_PAGE_SIZE;
		offset += FLASH_PAGE_SIZE;

		remaining -= FLASH_PAGE_SIZE;
	}

	if(remaining)
	{
		uint8_t data[FLASH_PAGE_SIZE];
		memcpy(data, ptr, remaining);

		flash_range_program(FLASH_TARGET_OFFSET + offset, ptr, FLASH_PAGE_SIZE);
	}

	restore_interrupts_from_disabled(ints);
	board_led_off();
}



// Get disk status
DSTATUS disk_status(BYTE pdrv)
{
	return 0;
}
// Initialize disk
DSTATUS disk_initialize(BYTE pdrv)
{
	return 0;
}

// Read sectors
DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
	if(sector + count > DISK_SECTOR_COUNT)
		return RES_PARERR;

	for(UINT i = 0; i < count; i++)
		memcpy(buff + i * DISK_SECTOR_SIZE, ram_disk.data[sector + i], DISK_SECTOR_SIZE);

	return RES_OK;
}

// Write sectors
DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count)
{
	if(sector + count > DISK_SECTOR_COUNT)
		return RES_PARERR;

	for(UINT i = 0; i < count; i++)
		memcpy(ram_disk.data[sector + i], buff + i * DISK_SECTOR_SIZE, DISK_SECTOR_SIZE);

	return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff)
{
	switch(cmd)
	{
		case CTRL_SYNC:
			return RES_OK;

		case GET_SECTOR_COUNT:
			*((LBA_t*)buff) = DISK_SECTOR_COUNT;
			return RES_OK;

		case GET_SECTOR_SIZE:
			*((WORD*)buff) = DISK_SECTOR_SIZE;
			return RES_OK;

		case GET_BLOCK_SIZE:
			*((DWORD*)buff) = 1; // Single sector erase (???)
			return RES_OK;

		default:
			return RES_PARERR;
	}
}

DWORD get_fattime()
{
	return ((DWORD)(FF_NORTC_YEAR - 1980) << 25 | (DWORD)FF_NORTC_MON << 21 | (DWORD)FF_NORTC_MDAY << 16);
}