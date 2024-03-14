add_library(cpp-yt-string)

target_compile_options(cpp-yt-string PRIVATE
  -Wdeprecated-this-capture
)

target_link_libraries(cpp-yt-string PUBLIC
  
  yutil
  cpp-yt-assert
  cpp-yt-exception
  cpp-yt-misc
  cpp-yt-small_containers
  cpp-string_utils-misc
)

target_sources(cpp-yt-string PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/string/enum.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/string/guid.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/string/string.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/string/format.cpp
)


