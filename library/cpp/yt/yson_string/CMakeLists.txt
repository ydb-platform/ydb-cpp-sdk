add_library(cpp-yt-yson_string)

target_link_libraries(cpp-yt-yson_string PUBLIC
  yutil
  cpp-yt-assert
  cpp-yt-coding
  cpp-yt-exception
  cpp-yt-string
  cpp-yt-memory
  cpp-yt-misc
)

target_sources(cpp-yt-yson_string PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/yson_string/convert.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/yson_string/string.cpp
)

if (NOT (WIN32 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64"))
  target_compile_options(cpp-yt-yson_string PRIVATE
    -Wdeprecated-this-capture
  )
endif ()
