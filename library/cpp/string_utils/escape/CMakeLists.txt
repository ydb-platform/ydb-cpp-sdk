add_library(cpp-string_utils-escape)

target_link_libraries(cpp-string_utils-escape PUBLIC
  contrib-libs-cxxsupp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-string_utils-escape PUBLIC
    contrib-libs-linux-headers
  )
endif()

target_sources(cpp-string_utils-escape PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/string_utils/escape/escape.cpp
)
