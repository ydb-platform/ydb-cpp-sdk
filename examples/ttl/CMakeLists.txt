add_executable(ttl)

target_link_libraries(ttl PUBLIC
  yutil
  library-cpp-getopt
  cpp-client-ydb_table
)

target_sources(ttl PRIVATE
  ${CMAKE_SOURCE_DIR}/examples/ttl/main.cpp
  ${CMAKE_SOURCE_DIR}/examples/ttl/ttl.cpp
)

vcs_info(ttl)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  target_link_libraries(ttl PUBLIC
    library-cpp-cpuid_check
  )
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(ttl PRIVATE
    -ldl
    -lrt
    -Wl,--no-as-needed
    -fPIC
    -lpthread
  )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_link_options(ttl PRIVATE
    -Wl,-platform_version,macos,11.0,11.0
    -fPIC
    -framework
    CoreFoundation
  )
endif()
