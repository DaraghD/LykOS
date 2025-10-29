#pragma once

#include <stdint.h>

// max sectors for lba28 = 268,435,456
#define ATA_BLK {"ATA BLK", 512, 268435456, 0, 0}

// not complete list - left out unused stuff

#define ATA_DATA_PORT 0x1F0
#define ATA_ERROR_PORT 0x1F1 // Error register (Read)
#define ATA_SECTOR_COUNT_PORT 0x1F2
#define ATA_LBA_LOW_PORT 0x1F3
#define ATA_LBA_MID_PORT 0x1F4
#define ATA_LBA_HIGH_PORT 0x1F5
#define ATA_DRIVE_SELECT_PORT 0x1F6
#define ATA_STATUS_PORT 0x1F7  // Status register (Read)
#define ATA_COMMAND_PORT 0x1F7 // Command register (Write)

#define ATA_STATUS_ERR 0x01
#define ATA_STATUS_DRQ 0x08
#define ATA_STATUS_BSY 0x80

#define ATA_CMD_READ_SECTORS 0x20
#define ATA_CMD_WRITE_SECTORS 0x30
#define ATA_CMD_FLUSH_CACHE 0xE7

#define ATA_DRIVE_MASTER 0xE0
#define ATA_IS_BUSY(status) ((status) & ATA_STATUS_BSY)
#define ATA_HAS_ERROR(status) ((status) & ATA_STATUS_ERR)

void ata_read_sector(uint32_t lba, uint8_t *buffer);
void ata_write_sector(uint32_t lba, uint8_t *buffer);
