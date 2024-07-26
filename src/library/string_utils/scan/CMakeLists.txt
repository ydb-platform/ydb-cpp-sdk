add_library(string_utils-scan)

target_link_libraries(string_utils-scan PUBLIC
  string_utils-helpers
)

target_sources(string_utils-scan PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/string_utils/scan/scan.cpp
)

_ydb_sdk_install_targets(TARGETS string_utils-scan)
