add_executable(enum_parser)

target_link_libraries(enum_parser PUBLIC
  yutil
  cpp-getopt-small
  tools-enum_parser-parse_enum
)

target_sources(enum_parser PRIVATE
  ${YDB_SDK_SOURCE_DIR}/tools/enum_parser/enum_parser/main.cpp
)

vcs_info(enum_parser)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  target_link_libraries(enum_parser PUBLIC
    library-cpp-cpuid_check
  )
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(enum_parser PRIVATE
    -ldl
    -lrt
    -Wl,--no-as-needed
    -lpthread
  )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_link_options(enum_parser PRIVATE
    -Wl,-platform_version,macos,11.0,11.0
    -framework
    CoreFoundation
  )
endif()