option(YDB_SDK_DOWNLOAD_PACKAGE_PROTOBUF "Download and setup Protobuf" ${YDB_SDK_DOWNLOAD_PACKAGES})
option(YDB_SDK_FORCE_DOWNLOAD_PROTOBUF "Download Protobuf even if there is an installed system package"
       ${YDB_SDK_FORCE_DOWNLOAD_PACKAGES}
)

set(YDB_SDK_PROTOBUF_VERSION 3.21.12)

if(NOT YDB_SDK_FORCE_DOWNLOAD_PROTOBUF)
    find_package(Protobuf QUIET)
endif()

if(NOT Protobuf_FOUND)
    message(STATUS "Could not find Protobuf package. Downloading and building from source.")

    include(DownloadUsingCPM)
    include(${CMAKE_CURRENT_LIST_DIR}/SetupAbseil.cmake)


    cpmaddpackage(
        NAME Protobuf
        VERSION ${YDB_SDK_PROTOBUF_VERSION}
        GITHUB_REPOSITORY protocolbuffers/protobuf
        OPTIONS
            "protobuf_BUILD_SHARED_LIBS OFF"
            "protobuf_BUILD_TESTS OFF"
            "protobuf_INSTALL OFF"
            "protobuf_ABSL_PROVIDER package"
    )

    if(NOT TARGET Protobuf::libprotobuf)
      add_library(Protobuf::libprotobuf ALIAS libprotobuf)
    endif()
    if(NOT TARGET Protobuf::libprotoc)
      add_library(Protobuf::libprotoc ALIAS libprotoc)
    endif()
    if(NOT TARGET Protobuf::protoc)
      add_executable(Protobuf::protoc ALIAS protoc)
    endif()
endif()

if(NOT Protobuf_VERSION)
    set(Protobuf_VERSION ${YDB_SDK_PROTOBUF_VERSION})
endif()


set(PROTOBUF_PROTOC $<TARGET_FILE:Protobuf::protoc>)

set(Protobuf_FOUND TRUE CACHE INTERNAL "Protobuf library is found")
message(STATUS "Using Protobuf version ${Protobuf_VERSION} (compiler: ${PROTOBUF_PROTOC})")