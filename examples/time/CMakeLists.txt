add_executable(time_example)

target_link_libraries(time_example
  PUBLIC
    yutil
    getopt
    YDB-CPP-SDK::Query
    YDB-CPP-SDK::Helpers
)

target_sources(time_example
  PRIVATE
    main.cpp
)

vcs_info(time_example)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  target_link_libraries(time_example PUBLIC
    cpuid_check
  )
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(time_example PRIVATE
    -ldl
    -lrt
    -Wl,--no-as-needed
    -lpthread
  )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_link_options(time_example PRIVATE
    -Wl,-platform_version,macos,11.0,11.0
    -framework
    CoreFoundation
  )
endif()
