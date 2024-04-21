add_library(cpp-monlib-exception)

target_link_libraries(cpp-monlib-exception PUBLIC
  yutil
)

target_sources(cpp-monlib-exception PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/exception/exception.cpp
)


