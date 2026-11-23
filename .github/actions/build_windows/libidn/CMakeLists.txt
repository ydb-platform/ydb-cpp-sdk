cmake_minimum_required(VERSION 3.22)
project(Libidn LANGUAGES C)

set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)
set(sources
  gl/basename-lgpl.c
  gl/fd-hook.c
  gl/getprogname.c
  gl/malloca.c
  gl/stat-time.c
  gl/stdlib.c
  lib/gl/c-ctype.c
  lib/gl/c-strcasecmp.c
  lib/gl/c-strncasecmp.c
  lib/gl/stdlib.c
  lib/gl/striconv.c
  lib/gl/strverscmp.c
  lib/gl/unistr/u8-check.c
  lib/gl/unistr/u8-mbtoucr.c
  lib/gl/unistr/u8-uctomb-aux.c
  lib/gl/unistr/u8-uctomb.c
  lib/idn-free.c
  lib/idna.c
  lib/nfkc.c
  lib/pr29.c
  lib/profiles.c
  lib/punycode.c
  lib/rfc3454.c
  lib/strerror-idna.c
  lib/strerror-pr29.c
  lib/strerror-punycode.c
  lib/strerror-stringprep.c
  lib/strerror-tld.c
  lib/stringprep.c
  lib/tld.c
  lib/tlds.c
  lib/toutf8.c
  lib/version.c
)
list(TRANSFORM sources PREPEND "${LIBIDN_SOURCE_DIR}/")
add_library(idn STATIC ${sources})
target_compile_definitions(idn PRIVATE HAVE_CONFIG_H LIBIDN_BUILDING LOCALEDIR=".")
target_include_directories(idn PRIVATE
  "${LIBIDN_SOURCE_DIR}/include"
  "${LIBIDN_SOURCE_DIR}"
  "${LIBIDN_SOURCE_DIR}/gl"
  "${LIBIDN_SOURCE_DIR}/lib"
  "${LIBIDN_SOURCE_DIR}/lib/gl"
)

# The MSVC headers do not declare the fallback implementation used by version.c.
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/strverscmp.h" "int strverscmp(const char*, const char*);\n")
set_property(SOURCE "${LIBIDN_SOURCE_DIR}/lib/version.c" APPEND PROPERTY
  COMPILE_OPTIONS "/FI${CMAKE_CURRENT_BINARY_DIR}/strverscmp.h")
