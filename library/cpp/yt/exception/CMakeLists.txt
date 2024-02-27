add_library(cpp-yt-exception)

target_compile_options(cpp-yt-exception PRIVATE
  -Wdeprecated-this-capture
)

target_link_libraries(cpp-yt-exception PUBLIC
  contrib-libs-cxxsupp
  yutil
)

target_sources(cpp-yt-exception PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/exception/exception.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-yt-exception PUBLIC
    contrib-libs-linux-headers
  )
endif()
