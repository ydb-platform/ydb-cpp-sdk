add_library(cppparser)

target_link_libraries(cppparser PUBLIC
  yutil
)

target_sources(cppparser PRIVATE
  parser.cpp
)

_ydb_sdk_install_targets(TARGETS cppparser)