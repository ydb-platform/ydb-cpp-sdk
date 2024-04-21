add_library(cpp-json-common)

target_link_libraries(cpp-json-common PUBLIC
  yutil
)

target_sources(cpp-json-common PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/json/common/defs.cpp
)


