add_executable(tracing_example tracing_example.cpp)

if(YDB_SDK_TRACING)
    target_link_libraries(tracing_example PRIVATE
            ydb-cpp-sdk
            ${OPENTELEMETRY_LIBRARIES}
    )
else()
    target_link_libraries(tracing_example PRIVATE ydb-cpp-sdk)
    target_compile_definitions(tracing_example PRIVATE -DYDB_SDK_TRACING_DISABLED)
endif()