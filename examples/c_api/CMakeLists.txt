add_executable(c_api_example
  main.c
)

target_link_libraries(c_api_example ydb-cpp-sdk::c-api)
