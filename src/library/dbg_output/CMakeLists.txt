add_library(library-dbg_output)

target_link_libraries(library-dbg_output PUBLIC
  yutil
  colorizer
  string_utils-misc
)

target_sources(library-dbg_output PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/dbg_output/dump.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/dbg_output/dumpers.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/dbg_output/engine.cpp
)

_ydb_sdk_install_targets(TARGETS library-dbg_output)
