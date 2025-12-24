add_executable(slo-key-value)

target_link_libraries(slo-key-value PUBLIC
  yutil
  getopt
  slo-utils
  YDB-CPP-SDK::Table
)

target_sources(slo-key-value PRIVATE
  create.cpp
  drop.cpp
  generate.cpp
  key_value.cpp
  main.cpp
  run.cpp
)

vcs_info(slo-key-value)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  target_link_libraries(slo-key-value PUBLIC
    cpuid_check
  )
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(slo-key-value PRIVATE
    -ldl
    -lrt
    -Wl,--no-as-needed
    -lpthread
  )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_link_options(slo-key-value PRIVATE
    -Wl,-platform_version,macos,11.0,11.0
    -framework
    CoreFoundation
  )
endif()
