# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: Copyright (c) 2019-2025 Lars Melchior and contributors

# Keep the bootstrap small and reviewable while pinning the downloaded CPM
# implementation by both version and digest.
set(CPM_DOWNLOAD_VERSION 0.42.0)
set(CPM_DOWNLOAD_SHA256
    2020b4fc42dba44817983e06342e682ecfc3d2f484a581f11cc5731fbe4dce8a)

if(CPM_SOURCE_CACHE)
  set(CPM_DOWNLOAD_LOCATION
      "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
  set(CPM_DOWNLOAD_LOCATION
      "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
  set(CPM_DOWNLOAD_LOCATION
      "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

get_filename_component(CPM_DOWNLOAD_LOCATION "${CPM_DOWNLOAD_LOCATION}" ABSOLUTE)
get_filename_component(_cpm_download_directory "${CPM_DOWNLOAD_LOCATION}" DIRECTORY)
file(MAKE_DIRECTORY "${_cpm_download_directory}")

set(_cpm_download_required ON)
if(EXISTS "${CPM_DOWNLOAD_LOCATION}")
  file(SHA256 "${CPM_DOWNLOAD_LOCATION}" _cpm_download_sha256)
  if(_cpm_download_sha256 STREQUAL CPM_DOWNLOAD_SHA256)
    set(_cpm_download_required OFF)
  endif()
endif()

if(_cpm_download_required)
  file(DOWNLOAD
    "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
    "${CPM_DOWNLOAD_LOCATION}"
    EXPECTED_HASH "SHA256=${CPM_DOWNLOAD_SHA256}"
    TLS_VERIFY ON
  )
endif()

unset(_cpm_download_required)
unset(_cpm_download_sha256)

include("${CPM_DOWNLOAD_LOCATION}")
