# add_subdirectory(ut)

add_library(ydb-library-security INTERFACE)

target_link_libraries(ydb-library-security INTERFACE
  yutil
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(ydb-library-security INTERFACE
    
  )
endif()
