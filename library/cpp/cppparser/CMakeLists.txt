add_library(library-cpp-cppparser)

target_link_libraries(library-cpp-cppparser PUBLIC
  contrib-libs-cxxsupp
  yutil
)

target_sources(library-cpp-cppparser PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/cppparser/parser.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(library-cpp-cppparser PUBLIC
    contrib-libs-linux-headers
  )
endif()
