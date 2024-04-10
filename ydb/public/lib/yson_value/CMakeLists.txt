add_library(public-lib-yson_value)
target_link_libraries(public-lib-yson_value PUBLIC
      yutil
    library-cpp-yson
    cpp-yson-node
    cpp-client-ydb_result
    cpp-client-ydb_value
    ydb-library-uuid
  )
if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
    target_link_libraries(public-lib-json_value PUBLIC
        
    )
endif()

target_sources(public-lib-yson_value PRIVATE
    ${YDB_SDK_SOURCE_DIR}/ydb/public/lib/yson_value/ydb_yson_value.cpp
)
