add_library(library-cpp-dbg_output)

target_link_libraries(library-cpp-dbg_output PUBLIC
  yutil
  library-cpp-colorizer
  cpp-string_utils-misc
)

target_sources(library-cpp-dbg_output PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/dbg_output/dump.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/dbg_output/dumpers.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/dbg_output/engine.cpp
)
