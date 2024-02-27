add_library(cpp-yt-assert)

target_compile_options(cpp-yt-assert PRIVATE
  -Wdeprecated-this-capture
)

target_link_libraries(cpp-yt-assert PUBLIC
  contrib-libs-linux-headers
  contrib-libs-cxxsupp
  yutil
)

target_sources(cpp-yt-assert PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/yt/assert/assert.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-yt-assert PUBLIC
    contrib-libs-linux-headers
  )
endif()
