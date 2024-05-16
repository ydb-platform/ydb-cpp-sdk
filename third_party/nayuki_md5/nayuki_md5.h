#pragma once

#include <stdint.h>

#if defined(__cplusplus)
extern "C"
#endif

void md5_compress(const uint8_t block[64], uint32_t state[4]);
