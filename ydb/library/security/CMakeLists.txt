# add_subdirectory(ut)

add_library(ydb-library-security)

target_link_libraries(ydb-library-security PUBLIC
  contrib-libs-cxxsupp
  yutil
  cpp-client-iam
  cpp-digest-crc32c
  client-ydb_types-credentials
  cpp-string_builder
)
target_sources(ydb-library-security PRIVATE
  ${CMAKE_SOURCE_DIR}/ydb/library/security/util.cpp
  ${CMAKE_SOURCE_DIR}/ydb/library/security/ydb_credentials_provider_factory.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(ydb-library-security PUBLIC
    contrib-libs-linux-headers
  )
endif()
