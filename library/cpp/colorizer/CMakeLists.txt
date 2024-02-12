add_library(library-cpp-colorizer)

target_link_libraries(library-cpp-colorizer PUBLIC
  contrib-libs-cxxsupp
  yutil
)

target_sources(library-cpp-colorizer PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/colorizer/colors.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/colorizer/output.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(library-cpp-colorizer PUBLIC
    contrib-libs-linux-headers
  )
endif()
