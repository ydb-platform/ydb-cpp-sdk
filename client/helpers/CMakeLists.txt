add_library(cpp-client-helpers)

target_link_libraries(cpp-client-helpers PUBLIC
  yutil
  cpp-client-iam
  client-ydb_types-credentials
  public-issue-protos
)

target_sources(cpp-client-helpers PRIVATE
  ${CMAKE_SOURCE_DIR}/client/helpers/helpers.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-client-helpers PUBLIC
    
  )
endif()
