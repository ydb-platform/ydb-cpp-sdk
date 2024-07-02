add_library(library-cppparser)

target_link_libraries(library-cppparser PUBLIC
  yutil
)

target_sources(library-cppparser PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/cppparser/parser.cpp
)

_ydb_sdk_install_targets(TARGETS library-cppparser)
