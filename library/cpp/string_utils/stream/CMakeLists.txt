add_library(cpp-string_utils-stream)

target_link_libraries(cpp-string_utils-stream PUBLIC
  yutil
)

target_sources(cpp-string_utils-stream PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/string_utils/stream/stream.cpp
)

