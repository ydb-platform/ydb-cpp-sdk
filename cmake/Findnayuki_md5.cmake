# - Find Nayuki_MD5
#
# nayuki_md5.h - Creating a header file for compatibility with the C standard
# NAYUKI_MD5_INCLUDE_DIR - Where to find LibNayuki_MD5 public headers
# NAYUKI_MD5_FOUND - True if Libnayuki_md5 found.

file(WRITE $ENV{NAYUKI_MD5_ROOT}/nayuki_md5.h
"#pragma once\n
#include <stdint.h>\n
#if defined(__cplusplus)
extern \"C\"
#endif\n
void md5_compress(uint32_t state[4], const uint8_t block[64]);\n"
)

file(READ $ENV{NAYUKI_MD5_ROOT}/nayuki_md5.c MD5_CURRENT)
string(REPLACE "#include <stdint.h>" "#include \"nayuki_md5.h\"" MD5_CURRENT "${MD5_CURRENT}")
string(REPLACE "void md5_compress(const uint8_t block[static 64], uint32_t state[static 4])" 
"void md5_compress(uint32_t state[4], const uint8_t block[64])" MD5_CURRENT "${MD5_CURRENT}")
file(WRITE "$ENV{NAYUKI_MD5_ROOT}/nayuki_md5.c" "${MD5_CURRENT}")

find_path(NAYUKI_MD5_INCLUDE_DIR
  "nayuki_md5.h"
  HINTS $ENV{NAYUKI_MD5_ROOT}
)

add_library(nayuki_md5)
target_link_libraries(nayuki_md5 PUBLIC
  yutil
)

target_sources(nayuki_md5 PRIVATE 
  $ENV{NAYUKI_MD5_ROOT}/nayuki_md5.c
)

include_directories(${NAYUKI_MD5_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(nayuki_md5 
DEFAULT_MSG 
NAYUKI_MD5_INCLUDE_DIR)

mark_as_advanced(NAYUKI_MD5_INCLUDE_DIR)