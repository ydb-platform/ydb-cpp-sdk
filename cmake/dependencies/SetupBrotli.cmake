option(YDB_SDK_DOWNLOAD_PACKAGE_BROTLI "Download and setup Brotli if no Brotli of matching version was found"
       ${YDB_SDK_DOWNLOAD_PACKAGES}
)

set(YDB_SDK_BROTLI_VERSION 1.1.0)

if(NOT YDB_SDK_FORCE_DOWNLOAD_PACKAGES)
    if(YDB_SDK_DOWNLOAD_PACKAGE_BROTLI)
        find_package(Brotli ${YDB_SDK_BROTLI_VERSION} QUIET)
    else()
        find_package(Brotli ${YDB_SDK_BROTLI_VERSION} REQUIRED)
    endif()

    if(Brotli_FOUND)
        return()
    endif()
endif()

include(DownloadUsingCPM)
cpmaddpackage(
    NAME
    Brotli
    VERSION
    ${YDB_SDK_BROTLI_VERSION}
    GITHUB_REPOSITORY
    google/brotli
    OPTIONS
    "BROTLI_DISABLE_TESTS TRUE"
    "BUILD_SHARED_LIBS OFF"
)

set(Brotli_FOUND TRUE)
set(Brotli_VERSION ${YDB_SDK_BROTLI_VERSION})
add_custom_target(Brotli)
write_package_stub(Brotli)

if(NOT TARGET Brotli::dec)
    add_library(Brotli::dec ALIAS brotlidec)
endif()
if(NOT TARGET Brotli::enc)
    add_library(Brotli::enc ALIAS brotlienc)
endif()