// Author: Yuxuan Zhang (github@z-yx.cc)
// =====================================
#include "crc.h"

extern const uint32_t *crc32_table;

uint32_t crc32(const void *buf, size_t len, uint32_t initial) {
  uint32_t code = initial ^ 0xFFFFFFFF;
  const uint8_t *u = (const uint8_t *)(buf);
  for (size_t i = 0; i < len; ++i) {
    code = crc32_table[(code ^ u[i]) & 0xFF] ^ (code >> 8);
  }
  return code ^ 0xFFFFFFFF;
}
