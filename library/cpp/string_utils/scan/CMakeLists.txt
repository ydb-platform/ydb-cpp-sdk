_ydb_sdk_add_library(string_utils-scan)

target_link_libraries(string_utils-scan 
  PUBLIC
    yutil
)

target_sources(string_utils-scan
  PRIVATE
    scan.cpp
)

_ydb_sdk_install_targets(TARGETS string_utils-scan)
