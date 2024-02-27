add_library(client-ydb_types-exceptions)

target_link_libraries(client-ydb_types-exceptions PUBLIC
  contrib-libs-linux-headers
  contrib-libs-cxxsupp
  yutil
)

target_sources(client-ydb_types-exceptions PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_types/exceptions/exceptions.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(client-ydb_types-exceptions PUBLIC
    contrib-libs-linux-headers
  )
endif()
