add_executable(pagination)

target_link_libraries(pagination PUBLIC
  yutil
  library-cpp-getopt
  cpp-client-ydb_table
)

target_sources(pagination PRIVATE
  ${CMAKE_SOURCE_DIR}/examples/pagination/main.cpp
  ${CMAKE_SOURCE_DIR}/examples/pagination/pagination_data.cpp
  ${CMAKE_SOURCE_DIR}/examples/pagination/pagination.cpp
)

vcs_info(pagination)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  target_link_libraries(pagination PUBLIC
    library-cpp-cpuid_check
  )
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(pagination PRIVATE
    -ldl
    -lrt
    -Wl,--no-as-needed
    -fPIC
    -lpthread
  )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_link_options(pagination PRIVATE
    -Wl,-platform_version,macos,11.0,11.0
    -fPIC
    -framework
    CoreFoundation
  )
endif()