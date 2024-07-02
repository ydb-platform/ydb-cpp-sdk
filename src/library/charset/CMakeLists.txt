add_subdirectory(lite)

add_library(library-charset)

target_link_libraries(library-charset PUBLIC
  yutil
  library-charset-lite
  Iconv::Iconv
)

target_sources(library-charset PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/charset/decodeunknownplane.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/charset/wide.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/charset/iconv.cpp
)

_ydb_sdk_install_targets(TARGETS library-charset)
