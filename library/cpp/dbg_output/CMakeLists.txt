add_library(library-cpp-dbg_output)

target_link_libraries(library-cpp-dbg_output PUBLIC
  contrib-libs-cxxsupp
  yutil
  library-cpp-colorizer
  cpp-string_utils-misc
)

target_sources(library-cpp-dbg_output PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/dbg_output/dump.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/dbg_output/dumpers.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/dbg_output/engine.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(library-cpp-dbg_output PUBLIC
    contrib-libs-linux-headers
  )
endif()
