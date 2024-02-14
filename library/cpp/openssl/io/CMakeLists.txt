add_library(cpp-openssl-io)

target_link_libraries(cpp-openssl-io PUBLIC
  contrib-libs-cxxsupp
  yutil
  certs
  OpenSSL::SSL
  cpp-openssl-init
  cpp-openssl-method
)

target_sources(cpp-openssl-io PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/openssl/io/stream.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-openssl-io PUBLIC
    contrib-libs-linux-headers
  )
endif()
