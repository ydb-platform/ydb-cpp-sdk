add_library(cpp-yt-yson)

target_compile_options(cpp-yt-yson PRIVATE
  -Wdeprecated-this-capture
)

target_link_libraries(cpp-yt-yson PUBLIC
  yutil
  cpp-yt-yson_string
)

target_sources(cpp-yt-yson PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/yson/consumer.cpp
)


