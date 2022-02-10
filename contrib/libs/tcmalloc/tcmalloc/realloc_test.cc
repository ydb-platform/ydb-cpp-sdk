// Copyright 2019 The TCMalloc Authors 
// 
// Licensed under the Apache License, Version 2.0 (the "License"); 
// you may not use this file except in compliance with the License. 
// You may obtain a copy of the License at 
// 
//     https://www.apache.org/licenses/LICENSE-2.0 
// 
// Unless required by applicable law or agreed to in writing, software 
// distributed under the License is distributed on an "AS IS" BASIS, 
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
// See the License for the specific language governing permissions and 
// limitations under the License. 
// 
// Test realloc() functionality 
 
#include <assert.h> 
#include <stddef.h> 
#include <stdint.h> 
#include <stdlib.h> 
#include <string.h> 
 
#include <algorithm> 
#include <utility> 
 
#include "gtest/gtest.h" 
#include "absl/random/random.h" 
#include "benchmark/benchmark.h"
 
namespace tcmalloc { 
namespace { 
 
// Fill a buffer of the specified size with a predetermined pattern 
void Fill(unsigned char* buffer, int n) { 
  for (int i = 0; i < n; i++) { 
    buffer[i] = (i & 0xff); 
  } 
} 
 
// Check that the specified buffer has the predetermined pattern 
// generated by Fill() 
void ExpectValid(unsigned char* buffer, int n) { 
  for (int i = 0; i < n; i++) { 
    ASSERT_EQ((i & 0xff), buffer[i]); 
  } 
} 
 
// Return the next interesting size/delta to check.  Returns -1 if no more. 
int NextSize(int size) { 
  if (size < 100) { 
    return size + 1; 
  } else if (size < 100000) { 
    // Find next power of two 
    int power = 1; 
    while (power < size) { 
      power <<= 1; 
    } 
 
    // Yield (power-1, power, power+1) 
    if (size < power - 1) { 
      return power - 1; 
    } else if (size == power - 1) { 
      return power; 
    } else { 
      assert(size == power); 
      return power + 1; 
    } 
  } else { 
    return -1; 
  } 
} 
 
TEST(ReallocTest, TestWithinCache) { 
  for (int src_size = 0; src_size >= 0; src_size = NextSize(src_size)) { 
    for (int dst_size = 0; dst_size >= 0; dst_size = NextSize(dst_size)) { 
      unsigned char* src = static_cast<unsigned char*>(malloc(src_size)); 
      Fill(src, src_size); 
      unsigned char* dst = static_cast<unsigned char*>(realloc(src, dst_size)); 
      ExpectValid(dst, std::min(src_size, dst_size)); 
      Fill(dst, dst_size); 
      ExpectValid(dst, dst_size); 
      if (dst != nullptr) free(dst); 
    } 
  } 
} 
 
TEST(ReallocTest, AlignedAllocRealloc) { 
  std::pair<size_t, size_t> sizes[] = {{1024, 2048}, {512, 128}}; 
 
  for (const auto& p : sizes) { 
    size_t src_size = p.first, dst_size = p.second; 
 
    auto src = static_cast<unsigned char*>(aligned_alloc(32, src_size)); 
    Fill(src, src_size); 
    auto dst = static_cast<unsigned char*>(realloc(src, dst_size)); 
    ExpectValid(dst, std::min(src_size, dst_size)); 
    Fill(dst, dst_size); 
    ExpectValid(dst, dst_size); 
    if (dst != nullptr) free(dst); 
  } 
} 
 
}  // namespace 
}  // namespace tcmalloc 
