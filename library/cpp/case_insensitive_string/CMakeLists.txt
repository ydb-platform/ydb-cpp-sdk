add_library(case_insensitive_string)

target_link_libraries(case_insensitive_string
  PUBLIC
    yutil
    contrib-libs-libc_compat
  PRIVATE
    digest-murmur
)

target_sources(case_insensitive_string 
  PRIVATE
    case_insensitive_char_traits.cpp
    case_insensitive_string.cpp
)

_ydb_sdk_install_targets(TARGETS case_insensitive_string)
