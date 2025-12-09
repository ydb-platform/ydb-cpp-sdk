option(YDB_SDK_DOWNLOAD_PACKAGE_BASE64 "Download and setup Base64 if no Base64 of matching version was found"
       ${YDB_SDK_DOWNLOAD_PACKAGES}
)

set(YDB_SDK_BASE64_VERSION 0.5.2)

if(NOT YDB_SDK_FORCE_DOWNLOAD_PACKAGES)
    if(YDB_SDK_DOWNLOAD_PACKAGE_BASE64)
        find_package(base64 QUIET)
    else()
        find_package(base64 REQUIRED)
    endif()

    if(base64_FOUND)
        return()
    endif()
endif()

include(DownloadUsingCPM)
cpmaddpackage(
    NAME base64
    VERSION ${YDB_SDK_BASE64_VERSION}
    GITHUB_REPOSITORY
    aklomp/base64
    OPTIONS
    "CMAKE_SKIP_INSTALL_RULES ON"
)
write_package_stub(base64)
add_library(aklomp::base64 ALIAS base64)
set(base64_FOUND TRUE)



