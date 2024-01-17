add_executable(validation)
target_link_libraries(validation PUBLIC
    contrib-libs-cxxsupp
    yutil
    contrib-libs-protoc
    api-protos-annotations
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND NOT HAVE_CUDA)
    target_link_libraries(validation PUBLIC
        contrib-libs-linux-headers
        library-cpp-cpuid_check
    )
    target_link_options(validation PRIVATE
        -ldl
        -lrt
        -Wl,--no-as-needed
        -fPIC
        -fPIC
        -lpthread
        -lrt
        -ldl
    )
    target_allocator(validation
        cpp-malloc-tcmalloc
        libs-tcmalloc-no_percpu_cache
    )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64" AND NOT HAVE_CUDA)
    target_link_libraries(validation PUBLIC
        contrib-libs-linux-headers
    )
    target_link_options(validation PRIVATE
        -ldl
        -lrt
        -Wl,--no-as-needed
        -fPIC
        -fPIC
        -lpthread
        -lrt
        -ldl
    )
    target_allocator(validation
        cpp-malloc-jemalloc
    )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    target_link_libraries(validation PUBLIC
        library-cpp-cpuid_check
    )
    target_link_options(validation PRIVATE
        -Wl,-platform_version,macos,11.0,11.0
        -fPIC
        -fPIC
    )
    target_allocator(validation
        system_allocator
    )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    target_link_options(validation PRIVATE
        -Wl,-platform_version,macos,11.0,11.0
        -fPIC
        -fPIC
    )
    target_allocator(validation
        system_allocator
    )
elseif (WIN32 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64" AND NOT HAVE_CUDA)
    target_link_libraries(validation PUBLIC
        library-cpp-cpuid_check
    )
    target_allocator(validation
        system_allocator
    )
endif()

vcs_info(validation)
