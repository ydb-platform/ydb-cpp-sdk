add_executable(secondary_index)

target_link_libraries(secondary_index PUBLIC
  yutil
  library-cpp-getopt
  YDB-CPP-SDK::Table
)

target_sources(secondary_index PRIVATE
  ${CMAKE_SOURCE_DIR}/examples/secondary_index/main.cpp
  ${CMAKE_SOURCE_DIR}/examples/secondary_index/secondary_index.cpp
  ${CMAKE_SOURCE_DIR}/examples/secondary_index/secondary_index_create.cpp
  ${CMAKE_SOURCE_DIR}/examples/secondary_index/secondary_index_delete.cpp
  ${CMAKE_SOURCE_DIR}/examples/secondary_index/secondary_index_drop.cpp
  ${CMAKE_SOURCE_DIR}/examples/secondary_index/secondary_index_generate.cpp
  ${CMAKE_SOURCE_DIR}/examples/secondary_index/secondary_index_list.cpp
  ${CMAKE_SOURCE_DIR}/examples/secondary_index/secondary_index_update.cpp
)

vcs_info(secondary_index)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  target_link_libraries(secondary_index PUBLIC
    library-cpp-cpuid_check
  )
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(secondary_index PRIVATE
    -ldl
    -lrt
    -Wl,--no-as-needed
    -fPIC
    -lpthread
  )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_link_options(secondary_index PRIVATE
    -Wl,-platform_version,macos,11.0,11.0
    -fPIC
    -framework
    CoreFoundation
  )
endif()
