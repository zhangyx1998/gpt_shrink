// Author: Yuxuan Zhang (github@z-yx.cc)
// =====================================
#pragma once

#include <stdint.h>
#include <unistd.h>

uint32_t crc32(const void *buf, size_t len, uint32_t initial);
