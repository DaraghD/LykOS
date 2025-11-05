#include "ata.h"
#include "arch/x86_64/io.h"
#include "drivers/serial.h"
#include <stdint.h>

static inline void ata_wait_ready(void) {
  while (inb(ATA_STATUS_PORT) & ATA_STATUS_BSY)
    ;
  while (!(inb(ATA_STATUS_PORT) & ATA_STATUS_DRQ))
    ;
}

// reads 512 bytes into buffer
void ata_read_sector(u32 lba, u8 *buffer) {
  outb(ATA_DRIVE_SELECT_PORT, ATA_DRIVE_MASTER | ((lba >> 24) & 0x0F));

  outb(ATA_SECTOR_COUNT_PORT, 1);
  outb(ATA_LBA_LOW_PORT, (u8)(lba & 0xFF));
  outb(ATA_LBA_MID_PORT, (u8)((lba >> 8) & 0xFF));
  outb(ATA_LBA_HIGH_PORT, (u8)((lba >> 16) & 0xFF));

  outb(ATA_COMMAND_PORT, ATA_CMD_READ_SECTORS);

  ata_wait_ready();

  u8 status = inb(ATA_STATUS_PORT);
  if (ATA_HAS_ERROR(status)) {
    u8 error = inb(ATA_ERROR_PORT);
    if (error & 0x01)
      serial_writeln("ATA error: Address Mark Not Found");
    else if (error & 0x02)
      serial_writeln("ATA error: Track 0 Not Found");
    else if (error & 0x04)
      serial_writeln("ATA error: Command Aborted");
    else if (error & 0x08)
      serial_writeln("ATA error: ID Not Found");
    else if (error & 0x10)
      serial_writeln("ATA error: Media Changed");
    else if (error & 0x20)
      serial_writeln("ATA error: Uncorrectable Data");
    else if (error & 0x40)
      serial_writeln("ATA error: Bad Block");
  }

  u16 *buf16 = (u16 *)buffer;
  for (int i = 0; i < 256; i++) {
    buf16[i] = inw(ATA_DATA_PORT);
  }
}

// write 512 bytes to buffer
void ata_write_sector(u32 lba, u8 *buffer) {
  outb(ATA_DRIVE_SELECT_PORT, ATA_DRIVE_MASTER | ((lba >> 24) & 0x0F));

  outb(ATA_SECTOR_COUNT_PORT, 1);
  outb(ATA_LBA_LOW_PORT, (u8)(lba & 0xFF));
  outb(ATA_LBA_MID_PORT, (u8)((lba >> 8) & 0xFF));
  outb(ATA_LBA_HIGH_PORT, (u8)((lba >> 16) & 0xFF));

  outb(ATA_COMMAND_PORT, ATA_CMD_WRITE_SECTORS);

  ata_wait_ready();

  u16 *buf16 = (u16 *)buffer;
  for (int i = 0; i < 256; i++) {
    outw(ATA_DATA_PORT, buf16[i]);
  }

  outb(ATA_COMMAND_PORT, ATA_CMD_FLUSH_CACHE);

  while (inb(ATA_STATUS_PORT) & ATA_STATUS_BSY)
    ;
}

// buffer must be big enough for count * sector size
void ata_read_sectors(u32 lba, u32 count, u16 *buffer) {}
void ata_write_sectors(u32 lba, u32 count, u16 *buffer) {}
