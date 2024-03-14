add_library(cpp-client-resources)
target_link_libraries(cpp-client-resources PUBLIC
  yutil
  library-cpp-resource
)
target_sources(cpp-client-resources PRIVATE
  ${CMAKE_SOURCE_DIR}/client/resources/ydb_resources.cpp
  ${CMAKE_SOURCE_DIR}/client/resources/ydb_ca.cpp
)

add_global_library_for(cpp-client-resources.global cpp-client-resources)
target_link_libraries(cpp-client-resources.global PUBLIC
  yutil
  library-cpp-resource
)
target_sources(cpp-client-resources.global PRIVATE
  ${CMAKE_BINARY_DIR}/client/resources/6ed212bf45019efe2a5e72b6d5ed50fb.cpp
)
resources(cpp-client-resources.global
  ${CMAKE_BINARY_DIR}/client/resources/6ed212bf45019efe2a5e72b6d5ed50fb.cpp
  INPUTS
  ${CMAKE_SOURCE_DIR}/client/resources/ydb_sdk_version.txt
  ${CMAKE_SOURCE_DIR}/client/resources/ydb_root_ca.pem
  KEYS
  ydb_sdk_version.txt
  ydb_root_ca.pem
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-client-resources PUBLIC
    
  )
  target_link_libraries(cpp-client-resources.global PUBLIC
    
  )
endif()
