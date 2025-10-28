cmake_policy(SET CMP0079 NEW)

option(YDB_SDK_DOWNLOAD_PACKAGE_GRPC "Download and setup gRPC" ${YDB_SDK_DOWNLOAD_PACKAGES})
option(YDB_SDK_FORCE_DOWNLOAD_GRPC "Download gRPC even if there is an installed system package"
       ${YDB_SDK_FORCE_DOWNLOAD_PACKAGES}
)

set(YDB_SDK_GRPC_VERSION 1.54.3)

macro(try_find_cmake_grpc)
    find_package(gRPC QUIET CONFIG)
    if(NOT gRPC_FOUND)
        find_package(gRPC QUIET)
    endif()

    if(gRPC_FOUND)
        # Use the found CMake-enabled gRPC package
        get_target_property(PROTO_GRPC_CPP_PLUGIN gRPC::grpc_cpp_plugin LOCATION)
    endif()
endmacro()

macro(try_find_system_grpc)
    # Find the system gRPC package
    set(GRPC_USE_SYSTEM_PACKAGE
        ON
        CACHE INTERNAL ""
    )

    if(YDB_SDK_DOWNLOAD_PACKAGE_GRPC)
        find_package(YdbSdkGrpc QUIET)
    else()
        find_package(YdbSdkGrpc REQUIRED)
    endif()

    if(YdbSdkGrpc_FOUND)
        set(gRPC_VERSION
            "${YdbSdkGrpc_VERSION}"
            CACHE INTERNAL ""
        )

        if(NOT TARGET gRPC::grpc++)
            add_library(gRPC::grpc++ ALIAS YdbSdkGrpc)
        endif()

        find_program(PROTO_GRPC_CPP_PLUGIN grpc_cpp_plugin)
        find_program(PROTO_GRPC_PYTHON_PLUGIN grpc_python_plugin)
    endif()
endmacro()

if(NOT YDB_SDK_FORCE_DOWNLOAD_GRPC)
    try_find_cmake_grpc()
    if(gRPC_FOUND)
        return()
    endif()

    try_find_system_grpc()
    if(YdbSdkGrpc_FOUND)
        return()
    endif()
endif()


# include(${CMAKE_CURRENT_LIST_DIR}/SetupAbseil.cmake)
# include(${CMAKE_CURRENT_LIST_DIR}/SetupCAres.cmake)
# include(${CMAKE_CURRENT_LIST_DIR}/SetupProtobuf.cmake)
include(DownloadUsingCPM)

set(YDB_SDK_GPRC_BUILD_FROM_SOURCE ON)

cpmaddpackage(
    NAME gRPC
    VERSION ${YDB_SDK_GRPC_VERSION}
    GITHUB_REPOSITORY
     grpc/grpc
    OPTIONS
    "BUILD_SHARED_LIBS OFF"
    "CARES_BUILD_TOOLS OFF"
    "RE2_BUILD_TESTING OFF"
    "OPENSSL_NO_ASM ON"
    "gRPC_BUILD_TESTS OFF"
    "gRPC_BUILD_GRPC_NODE_PLUGIN OFF"
    "gRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN OFF"
    "gRPC_BUILD_GRPC_PHP_PLUGIN OFF"
    "gRPC_BUILD_GRPC_RUBY_PLUGIN OFF"
    "gRPC_BUILD_GRPC_CSHARP_PLUGIN OFF"
    "gRPC_ZLIB_PROVIDER package"
    "gRPC_CARES_PROVIDER package"
    "gRPC_RE2_PROVIDER package"
    "gRPC_SSL_PROVIDER package"
    "gRPC_PROTOBUF_PROVIDER package"
    "gRPC_BENCHMARK_PROVIDER none"
    "gRPC_ABSL_PROVIDER package"
    "gRPC_CARES_LIBRARIES c-ares::cares"
    "gRPC_INSTALL OFF"
    "gRPC_BUILD_GRPC_PYTHON_PLUGIN OFF"
)

set(gRPC_VERSION "${YDB_SDK_GRPC_VERSION}")
set(PROTO_GRPC_CPP_PLUGIN $<TARGET_FILE:grpc_cpp_plugin>)

write_package_stub(gRPC)
if(NOT TARGET "gRPC::grpc++")
    add_library(gRPC::grpc++ ALIAS grpc++)
endif()
if(NOT TARGET "gRPC::grpc_cpp_plugin")
    add_executable(gRPC::grpc_cpp_plugin ALIAS grpc_cpp_plugin)
endif()
if(NOT TARGET "gRPC::grpcpp_channelz")
    add_library(gRPC::grpcpp_channelz ALIAS grpcpp_channelz)
endif()
mark_targets_as_system("${gRPC_SOURCE_DIR}")
