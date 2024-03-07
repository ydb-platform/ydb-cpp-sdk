add_library(cpp-string_utils-url)

target_link_libraries(cpp-string_utils-url PUBLIC
  contrib-libs-cxxsupp
  yutil
  cpp-string_utils-misc
)

target_sources(cpp-string_utils-url PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/string_utils/url/url.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-string_utils-url PUBLIC
    contrib-libs-linux-headers
  )
endif()
