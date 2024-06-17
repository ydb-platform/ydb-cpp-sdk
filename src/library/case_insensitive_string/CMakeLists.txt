add_library(library-case_insensitive_string)

target_link_libraries(library-case_insensitive_string PUBLIC
  yutil
  digest-murmur
)

target_sources(library-case_insensitive_string PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/case_insensitive_string/case_insensitive_char_traits.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/case_insensitive_string/case_insensitive_string.cpp
)

_ydb_sdk_install_targets(TARGETS library-case_insensitive_string)
