# add_subdirectory(ut)

add_library(public-lib-json_value)

target_link_libraries(public-lib-json_value PUBLIC
      yutil
    cpp-json-writer
    cpp-string_utils-base64
    cpp-client-ydb_result
    cpp-client-ydb_value
    ydb-library-uuid
    cpp-string_builder 
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
    target_link_libraries(public-lib-json_value PUBLIC
        
    )
endif()

target_sources(public-lib-json_value PRIVATE
    ${CMAKE_SOURCE_DIR}/ydb/public/lib/json_value/ydb_json_value.cpp
)
