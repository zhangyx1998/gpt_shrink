// Author: Yuxuan Zhang (github@z-yx.cc)
// =====================================
#pragma once

#include "crc.h"
#include <stdint.h>

struct gpt_header {
  uint8_t signature[8];
  uint32_t revision;
  uint32_t header_size;
  uint32_t header_crc32;
  uint32_t reserved;
  uint64_t current_lba;
  uint64_t backup_lba;
  uint64_t first_usable_lba;
  uint64_t last_usable_lba;
  uint8_t disk_guid[16];
  uint64_t partition_entry_lba;
  uint32_t num_partition_entries;
  uint32_t partition_entry_size;
  uint32_t partition_entry_array_crc32;
};

struct gpt_entry {
  uint8_t type_guid[16];
  uint8_t partition_guid[16];
  uint64_t first_lba;
  uint64_t last_lba;
  uint8_t attributes[8];
  uint16_t name[36];
};

static inline uint32_t header_crc(struct gpt_header *source_header) {
  struct gpt_header header = *source_header;
  header.header_crc32 = 0;
  return crc32(&header, 0x5c, 0);
}
