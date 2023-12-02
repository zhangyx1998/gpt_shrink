// Author: Yuxuan Zhang (github@z-yx.cc)
// =====================================
#include "log.h"
#include "crc.h"
#include <string.h>

int log_gpt_header(struct gpt_header *header) {
  LOG("signature: %.8s\n", header->signature);
  LOG("revision: %u\n", header->revision);
  LOG("header_size: %u\n", header->header_size);
  LOG("header_crc32: %u\n", header->header_crc32);
  LOG("reserved: %u\n", header->reserved);
  LOG("current_lba: %lu\n", header->current_lba);
  LOG("backup_lba: %lu\n", header->backup_lba);
  LOG("first_usable_lba: %lu\n", header->first_usable_lba);
  LOG("last_usable_lba: %lu\n", header->last_usable_lba);
  LOG("disk_guid: ");
  for (int i = 0; i < 16; i++) {
    if (i && i % 4 == 0)
      LOG("-");
    LOG("%02x", header->disk_guid[i]);
  }
  LOG("\n");
  LOG("partition_entry_lba: %lu\n", header->partition_entry_lba);
  LOG("num_partition_entries: %u\n", header->num_partition_entries);
  LOG("partition_entry_size: %u\n", header->partition_entry_size);
  LOG("partition_entry_array_crc32: %u\n", header->partition_entry_array_crc32);
  int ret = 0;
  if (memcmp(header->signature, "EFI PART", 8)) {
    LOG("########## Invalid GPT signature ##########\n");
    ret = - 1;
  }
  const uint32_t crc = header_crc(header);
  if (crc != header->header_crc32) {
    LOG("########## Header CRC mismatch ##########\n");
    LOG("Provided CRC32 = %08x\n", header->header_crc32);
    LOG("Computed CRC32 = %08x\n", crc);
    ret = -2;
  } else {
    LOG("# Header CRC OK\n");
  }

  return ret;
}
