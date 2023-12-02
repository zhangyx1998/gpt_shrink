// Author: Yuxuan Zhang (github@z-yx.cc)
// =====================================
#pragma once

#include "gpt.h"
#include <stdio.h>

#define LOG(...) fprintf(stderr, __VA_ARGS__)

int log_gpt_header(struct gpt_header *header);
