add_executable(basic_example)

target_link_libraries(basic_example PUBLIC
  yutil
  library-cpp-getopt
  YDB-CPP-SDK::Table
)

target_sources(basic_example PRIVATE
  ${CMAKE_SOURCE_DIR}/examples/basic_example/main.cpp
  ${CMAKE_SOURCE_DIR}/examples/basic_example/basic_example_data.cpp
  ${CMAKE_SOURCE_DIR}/examples/basic_example/basic_example.cpp
)

vcs_info(basic_example)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  target_link_libraries(basic_example PUBLIC
    library-cpp-cpuid_check
  )
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(basic_example PRIVATE
    -ldl
    -lrt
    -Wl,--no-as-needed
    -lpthread
  )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_link_options(basic_example PRIVATE
    -Wl,-platform_version,macos,11.0,11.0
    -framework
    CoreFoundation
  )
endif()
