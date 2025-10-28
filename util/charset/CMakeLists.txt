if (YDB_SDK_TESTS)
  add_subdirectory(ut)
endif()

_ydb_sdk_add_library(util-charset)

target_compile_options(util-charset PUBLIC
  -DTSTRING_IS_STD_STRING
  -DNO_CUSTOM_CHAR_PTR_STD_COMPARATOR
)

target_compile_options(util-charset PRIVATE
  -Wnarrowing
)

target_joined_source(util-charset
  all_charset.cpp
  ${YDB_SDK_SOURCE_DIR}/util/charset/generated/unidata.cpp
  ${YDB_SDK_SOURCE_DIR}/util/charset/recode_result.cpp
  ${YDB_SDK_SOURCE_DIR}/util/charset/unicode_table.cpp
  ${YDB_SDK_SOURCE_DIR}/util/charset/unidata.cpp
  ${YDB_SDK_SOURCE_DIR}/util/charset/utf8.cpp
  ${YDB_SDK_SOURCE_DIR}/util/charset/wide.cpp
)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  target_sources_custom(util-charset
    .sse41
    SRCS
    ${YDB_SDK_SOURCE_DIR}/util/charset/wide_sse41.cpp
    CUSTOM_FLAGS
    -msse4.1
  )
elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
  target_sources(util-charset PRIVATE
    ${YDB_SDK_SOURCE_DIR}/util/charset/wide_sse41.cpp
  )
  set_property(
    SOURCE
    ${YDB_SDK_SOURCE_DIR}/util/charset/wide_sse41.cpp
    APPEND
    PROPERTY
    COMPILE_OPTIONS
    -DSSE41_STUB
  )
endif()

_ydb_sdk_install_targets(TARGETS util-charset)
