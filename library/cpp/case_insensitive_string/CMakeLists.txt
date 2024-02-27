add_library(library-cpp-case_insensitive_string)

target_link_libraries(library-cpp-case_insensitive_string PUBLIC
  contrib-libs-cxxsupp
  yutil
  contrib-libs-libc_compat
  cpp-digest-murmur
)

target_sources(library-cpp-case_insensitive_string PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/case_insensitive_string/case_insensitive_char_traits.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/case_insensitive_string/case_insensitive_string.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(library-cpp-case_insensitive_string PUBLIC
    contrib-libs-linux-headers
  )
endif()
