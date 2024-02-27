add_library(ydb-library-yverify_stream INTERFACE)

target_link_libraries(ydb-library-yverify_stream INTERFACE
  contrib-libs-cxxsupp
  yutil
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(ydb-library-yverify_stream INTERFACE
    contrib-libs-linux-headers
  )
endif()
