add_library(cgiparam)

target_link_libraries(cgiparam 
  PUBLIC
    yutil
    iterator
  PRIVATE
    string_utils-quote
    string_utils-scan
)

target_sources(cgiparam PRIVATE
  cgiparam.cpp
)

_ydb_sdk_install_targets(TARGETS cgiparam)
