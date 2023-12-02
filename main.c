// Author: Yuxuan Zhang (github@z-yx.cc)
// =====================================
// Main source file for gpt_shrink
#include "gpt.h"
#include "log.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct gpt_header primary, backup, header;

#define SEP "\n====================\n"

static size_t total_output_sectors = 0;

static inline void update_progress() {
  LOG("\033[u\033[s\033[K %.02f GB",
      (double)total_output_sectors / (double)(1 << (30 - 9)));
}

void output_sector(int fd, size_t sector) {
  static char buf[512];
  lseek(fd, sector * 512, SEEK_SET);
  read(fd, buf, 512);
  write(fileno(stdout), buf, 512);
  total_output_sectors++;
}

void output_align_sector(void *data, size_t len) {
  write(fileno(stdout), data, len);
  total_output_sectors += len / 512;
  if (len % 512) {
    // Fill the rest of a remaining sector with zeros
    char buf[512];
    size_t match_len = 512 - len % 512;
    memset(buf, 0, 512);
    write(fileno(stdout), buf, match_len);
    total_output_sectors++;
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    LOG("Usage: %s <file>\n", argv[0]);
    return -1;
  }
  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    perror("open");
    return -1;
  }

  // Seek to sector 1 and read the primary GPT header
  lseek(fd, 512, SEEK_SET);
  read(fd, &primary, sizeof(primary));
  LOG(SEP "Primary GPT header:" SEP);
  if (log_gpt_header(&primary))
    return -1;

  // Seek to the last sector and read the backup GPT header
  lseek(fd, primary.backup_lba * 512, SEEK_SET);
  read(fd, &backup, sizeof(backup));
  LOG(SEP "Backup GPT header:" SEP);
  if (log_gpt_header(&backup))
    return -1;

  header = primary;
  const size_t table_size =
                   header.num_partition_entries * header.partition_entry_size,
               table_sectors = table_size / 512 + (table_size % 512 ? 1 : 0);
  uint8_t *table_primary = (uint8_t *)malloc(table_size),
          *table_backup = (uint8_t *)malloc(table_size);
  // Read all partition entries from the primary GPT
  lseek(fd, header.partition_entry_lba * 512, SEEK_SET);
  read(fd, table_primary, table_size);
  // Read all partition entries from the backup GPT
  lseek(fd, backup.partition_entry_lba * 512, SEEK_SET);
  read(fd, table_backup, table_size);
  // Record the last used data sector
  size_t last_data_sector = 0;
  // Seek to each of the partition entries in both primary and backup GPT,
  // compare and print
  for (unsigned int i = 0; i < header.num_partition_entries; i++) {
    const struct gpt_entry *entry =
        (struct gpt_entry *)(table_primary + i * header.partition_entry_size);
    // Check for empty GUID entry
    int flag_empty = 1;
    for (int j = 0; j < 16; j++) {
      if (entry->type_guid[j]) {
        flag_empty = 0;
        break;
      }
    }
    if (!flag_empty) {
      static char name[37];
      memset(name, 0, 37);
      for (unsigned j = 0; j < 36; j++) {
        if (entry->name[j] == 0)
          continue;
        name[j] = (char)entry->name[j];
      }
      LOG(SEP "Partition %u: %s" SEP, i, name);
      LOG("type_guid : ");
      for (int i = 0; i < 16; i++) {
        if (i && i % 4 == 0)
          LOG("-");
        LOG("%02x", entry->type_guid[i]);
      }
      LOG("\n");
      LOG("part_guid : ");
      for (int i = 0; i < 16; i++) {
        if (i && i % 4 == 0)
          LOG("-");
        LOG("%02x", entry->partition_guid[i]);
      }
      LOG("\n");
      LOG("sectors   : %lu ~ %lu\n", entry->first_lba, entry->last_lba);
      LOG("attributes:");
      for (int i = 0; i < 8; i++) {
        LOG(" %02x", entry->attributes[i]);
      }
      LOG("\n");
    }
    // Update last_data_sector
    if (entry->last_lba > last_data_sector)
      last_data_sector = entry->last_lba;
    // Compare with the backup GPT
    if (memcmp(entry, table_backup + i * header.partition_entry_size,
               sizeof(*entry))) {
      LOG(SEP "Partition Table Mismatch" SEP);
      close(fd);
      return -1;
    }
  }
  LOG(SEP "Last Data Sector: %lu" SEP, last_data_sector);
  header.last_usable_lba = last_data_sector;
  header.backup_lba = last_data_sector + table_sectors + 1;
  header.header_crc32 = header_crc(&header);
  // Output new disk binary to stdout
  if (!isatty(fileno(stdout))) {
    LOG("\nWriting new binary: \033[s");
    output_sector(fd, 0);
    output_align_sector(&header, sizeof(header));
    output_align_sector(table_primary, table_size);
    unsigned int counter = 0;
    while (total_output_sectors <= last_data_sector) {
      output_sector(fd, total_output_sectors);
      if (++counter == 10000) {
        counter = 0;
        update_progress();
      }
    }
    // Finish up
    LOG(SEP "Writing backup GPT" SEP);
    LOG("Sectors expected: %lu\n", last_data_sector + 1);
    LOG("Sectors written : %lu\n", total_output_sectors);
    // backup GPT entries
    output_align_sector(table_primary, table_size);
    // swap primary and backup lba pointers
    const uint64_t backup_lba = header.backup_lba;
    header.backup_lba = header.current_lba;
    header.current_lba = backup_lba;
    // Update partition entry array pointer
    header.partition_entry_lba = last_data_sector + 1;
    // Update CRC32
    header.header_crc32 = header_crc(&header);
    // Write backup GPT header
    output_align_sector(&header, sizeof(header));
    LOG("\n" SEP "DONE" SEP);
  } else {
    LOG("\nSTDOUT is NOT redirected to a file, aborting.\n");
  }
  close(fd);
  return 0;
}
