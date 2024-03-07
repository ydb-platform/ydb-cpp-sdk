add_library(cpp-string_utils-helpers)

target_link_libraries(cpp-string_utils-helpers PUBLIC
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-string_utils-helpers PUBLIC
    
  )
endif()

target_sources(cpp-string_utils-helpers PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/string_utils/helpers/helpers.cpp
)
