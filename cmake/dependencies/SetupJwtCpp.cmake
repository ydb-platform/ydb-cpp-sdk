option(YDB_SDK_DOWNLOAD_PACKAGE_JWT_CPP "Download and setup jwt-cpp if no jwt-cpp of matching version was found"
       ${YDB_SDK_DOWNLOAD_PACKAGES}
)

set(YDB_SDK_JWT_CPP_VERSION 0.7.0)

if(NOT YDB_SDK_FORCE_DOWNLOAD_PACKAGES)
    if(YDB_SDK_DOWNLOAD_PACKAGE_JWT_CPP)
        find_package(jwt-cpp QUIET)
    else()
        find_package(jwt-cpp REQUIRED)
    endif()

    if(jwt-cpp_FOUND)
        return()
    endif()
endif()

include(DownloadUsingCPM)
cpmaddpackage(
    NAME
    jwt-cpp
    VERSION
    ${YDB_SDK_JWT_CPP_VERSION}
    GITHUB_REPOSITORY
    Thalhammer/jwt-cpp
    OPTIONS
    "JWT_BUILD_EXAMPLES OFF"
    "CMAKE_SKIP_INSTALL_RULES ON"
)
write_package_stub(jwt-cpp)
set(jwt-cpp_FOUND TRUE)
