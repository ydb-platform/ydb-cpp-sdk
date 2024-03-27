add_library(util-charset)

target_joined_source(util-charset
  all_charset.cpp
  ${CMAKE_SOURCE_DIR}/util/charset/generated/unidata.cpp
  ${CMAKE_SOURCE_DIR}/util/charset/recode_result.cpp
  ${CMAKE_SOURCE_DIR}/util/charset/unicode_table.cpp
  ${CMAKE_SOURCE_DIR}/util/charset/unidata.cpp
  ${CMAKE_SOURCE_DIR}/util/charset/utf8.cpp
  ${CMAKE_SOURCE_DIR}/util/charset/wide.cpp
)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  target_sources_custom(util-charset
    .sse41
    SRCS
    ${CMAKE_SOURCE_DIR}/util/charset/wide_sse41.cpp
    CUSTOM_FLAGS
    -msse4.1
  )
elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
  target_sources(util-charset PRIVATE
    ${CMAKE_SOURCE_DIR}/util/charset/wide_sse41.cpp
  )
  set_property(
    SOURCE
    ${CMAKE_SOURCE_DIR}/util/charset/wide_sse41.cpp
    APPEND
    PROPERTY
    COMPILE_OPTIONS
    -DSSE41_STUB
  )
endif()
