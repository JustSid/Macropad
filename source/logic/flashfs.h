//
// Created by Sidney on 22/08/2025.
//

#ifndef MACROPAD_FLASHFS_H
#define MACROPAD_FLASHFS_H

#define DISK_SECTOR_COUNT 128  // 128 sectors @ 512 bytes each = 64KB
#define DISK_SECTOR_SIZE  512

void flashfs_init();

void flashfs_read(void *buffer, uint32_t lba, uint32_t offset, uint32_t length);
void flashfs_write(void *buffer, uint32_t lba, uint32_t offset, uint32_t length);

void flashfs_flush();

#endif //MACROPAD_FLASHFS_H