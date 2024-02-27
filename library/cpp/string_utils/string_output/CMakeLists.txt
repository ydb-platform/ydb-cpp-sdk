add_library(cpp-string_utils-string_output)

target_link_libraries(cpp-string_utils-string_output PUBLIC
  contrib-libs-cxxsupp
  yutil
)

target_sources(cpp-string_utils-string_output PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/string_utils/string_output/string_output.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-string_utils-string_output PUBLIC
    contrib-libs-linux-headers
  )
endif()