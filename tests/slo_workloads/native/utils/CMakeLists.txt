include(FetchContent)

FetchContent_Declare(
  hdr_histogram
  GIT_REPOSITORY https://github.com/HdrHistogram/HdrHistogram_c.git
  GIT_TAG        0.11.8
  EXCLUDE_FROM_ALL
)
set(HDR_HISTOGRAM_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(HDR_HISTOGRAM_BUILD_SHARED   OFF CACHE BOOL "" FORCE)
set(HDR_LOG_REQUIRED             OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(hdr_histogram)

add_library(slo-utils)

target_link_libraries(slo-utils PUBLIC
  slo-workload-core
  yutil
  getopt
  YDB-CPP-SDK::Table
  YDB-CPP-SDK::Iam
)

target_link_libraries(slo-utils PRIVATE
  hdr_histogram_static
)

target_sources(slo-utils PRIVATE
  executor.cpp
  job.cpp
  utils.cpp
)
