add_library(cpp-string_builder)

target_link_libraries(cpp-string_builder PUBLIC
  contrib-libs-cxxsupp
  yutil
  cpp-string_utils-string_output
)

target_sources(cpp-string_builder PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/string_builder/string_builder.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-string_builder PUBLIC
    contrib-libs-linux-headers
  )
endif()