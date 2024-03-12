add_library(cpp-string_utils-scan)

target_link_libraries(cpp-string_utils-scan PUBLIC
  yutil
)

target_sources(cpp-string_utils-scan PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/string_utils/scan/scan.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-string_utils-scan PUBLIC
    
  )
endif()
