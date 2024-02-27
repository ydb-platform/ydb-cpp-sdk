add_library(cpp-monlib-exception)

target_link_libraries(cpp-monlib-exception PUBLIC
  contrib-libs-cxxsupp
  yutil
)

target_sources(cpp-monlib-exception PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/exception/exception.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-monlib-exception PUBLIC
    contrib-libs-linux-headers
  )
endif()
