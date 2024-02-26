add_library(cpp-string_utils-misc)

target_link_libraries(cpp-string_utils-misc PUBLIC
  contrib-libs-cxxsupp
  yutil
  cpp-string_utils-escape
  cpp-string_builder
  yutil
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-string_utils-misc PUBLIC
    contrib-libs-linux-headers
  )
endif()

target_sources(cpp-string_utils-misc PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/string_utils/misc/misc.cpp
)
