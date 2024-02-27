add_subdirectory(lite)

add_library(library-cpp-charset)

target_link_libraries(library-cpp-charset PUBLIC
  contrib-libs-cxxsupp
  yutil
  cpp-charset-lite
  Iconv::Iconv
)

target_sources(library-cpp-charset PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/charset/decodeunknownplane.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/charset/wide.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/charset/iconv.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(library-cpp-charset PUBLIC
    contrib-libs-linux-headers
  )
endif()
