add_library(cpp-yt-misc)

target_compile_options(cpp-yt-misc PRIVATE
  -Wdeprecated-this-capture
)

target_link_libraries(cpp-yt-misc PUBLIC
  yutil
  cpp-yt-exception
  tools-enum_parser-enum_serialization_runtime
  cpp-string_utils-misc
)

target_sources(cpp-yt-misc PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/misc/guid.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/misc/source_location.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/misc/thread_name.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/misc/variant.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-yt-misc PUBLIC
    
  )
endif()
