add_library(library-json-common)

target_link_libraries(library-json-common PUBLIC
  yutil
)

target_sources(library-json-common PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/json/common/defs.cpp
)

_ydb_sdk_install_targets(TARGETS library-json-common)
