add_library(cpp-string_builder INTERFACE)

target_link_libraries(cpp-string_builder INTERFACE
  contrib-libs-cxxsupp
  yutil
  cpp-string_utils-string_output
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-string_builder INTERFACE
    contrib-libs-linux-headers
  )
endif()