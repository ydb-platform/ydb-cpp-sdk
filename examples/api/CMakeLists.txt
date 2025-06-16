add_executable(api_example_sync sync.cpp)

target_link_libraries(api_example_sync PRIVATE YDB-CPP-SDK::Params YDB-CPP-SDK::Types)

add_executable(api_example_async async.cpp)

target_link_libraries(api_example_async PRIVATE YDB-CPP-SDK::Params YDB-CPP-SDK::Types)
