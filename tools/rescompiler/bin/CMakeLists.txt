add_executable(rescompiler)

target_link_libraries(rescompiler PUBLIC
  yutil
  library-cpp-resource
)

target_sources(rescompiler PRIVATE
  ${CMAKE_SOURCE_DIR}/tools/rescompiler/main.cpp
)

vcs_info(rescompiler)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  target_link_libraries(rescompiler PUBLIC
    library-cpp-cpuid_check
  )
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(rescompiler PRIVATE
    -ldl
    -lrt
    -Wl,--no-as-needed
    -lpthread
  )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_link_options(rescompiler PRIVATE
    -Wl,-platform_version,macos,11.0,11.0
    -framework
    CoreFoundation
  )
endif()
