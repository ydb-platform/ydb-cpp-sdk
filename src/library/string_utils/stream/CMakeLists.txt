add_library(cpp-string_utils-stream)

target_link_libraries(cpp-string_utils-stream PUBLIC
  yutil
)

target_sources(cpp-string_utils-stream PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/string_utils/stream/stream.cpp
)

