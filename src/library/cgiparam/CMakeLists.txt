add_library(library-cgiparam)

target_link_libraries(library-cgiparam PUBLIC
  yutil
  library-iterator
  string_utils-quote
  string_utils-scan
)

target_sources(library-cgiparam PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/cgiparam/cgiparam.cpp
)

_ydb_sdk_install_targets(TARGETS library-cgiparam)
