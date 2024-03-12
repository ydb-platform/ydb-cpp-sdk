add_library(cpp-http-simple)

target_link_libraries(cpp-http-simple PUBLIC
  yutil
  cpp-http-io
  cpp-openssl-io
  cpp-string_utils-url
  library-cpp-uri
)

target_sources(cpp-http-simple PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/http/simple/http_client.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-http-simple PUBLIC
    
  )
endif()
