add_library(cpp-http-server)

target_link_libraries(cpp-http-server PUBLIC
  yutil
  cpp-http-misc
  cpp-http-io
  cpp-threading-equeue
  cpp-deprecated-atomic
)

target_sources(cpp-http-server PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/http/server/conn.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/http/server/http.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/http/server/http_ex.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/http/server/options.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/http/server/response.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-http-server PUBLIC
    
  )
endif()
