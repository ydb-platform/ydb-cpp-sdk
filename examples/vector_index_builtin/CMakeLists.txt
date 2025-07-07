add_executable(vector_index_builtin)

target_link_libraries(vector_index_builtin
  PUBLIC
    yutil
    getopt
    YDB-CPP-SDK::Query
    YDB-CPP-SDK::Table
    YDB-CPP-SDK::Helpers
)

target_sources(vector_index_builtin
  PRIVATE
    main.cpp
    vector_index.cpp
)

vcs_info(vector_index_builtin)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  target_link_libraries(vector_index_builtin PUBLIC
    cpuid_check
  )
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(vector_index_builtin PRIVATE
    -ldl
    -lrt
    -Wl,--no-as-needed
    -lpthread
  )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_link_options(vector_index_builtin PRIVATE
    -Wl,-platform_version,macos,11.0,11.0
    -framework
    CoreFoundation
  )
endif()
