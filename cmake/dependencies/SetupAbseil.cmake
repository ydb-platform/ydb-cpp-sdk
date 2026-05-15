option(YDB_SDK_DOWNLOAD_PACKAGE_ABSEIL "Download and setup Abseil if no Abseil matching version was found"
       ${YDB_SDK_DOWNLOAD_PACKAGES}
)
option(YDB_SDK_FORCE_DOWNLOAD_ABSEIL "Download Abseil even if it exists in a system" ${YDB_SDK_DOWNLOAD_PACKAGES})

if(NOT YDB_SDK_FORCE_DOWNLOAD_ABSEIL)
    set(ABSL_PROPAGATE_CXX_STD ON)

    if(YDB_SDK_DOWNLOAD_PACKAGE_ABSEIL)
        find_package(absl QUIET)
    else()
        find_package(absl REQUIRED)
    endif()

    if(absl_FOUND)
        return()
    endif()
endif()

include(DownloadUsingCPM)

cpmaddpackage(
    NAME
    abseil-cpp
    VERSION
    20230802.0
    GIT_TAG
    20230802.0
    GITHUB_REPOSITORY
    abseil/abseil-cpp
    OPTIONS
    "ABSL_PROPAGATE_CXX_STD ON"
    "ABSL_ENABLE_INSTALL ON"
)

mark_targets_as_system("${abseil-cpp_SOURCE_DIR}")
write_package_stub(absl)