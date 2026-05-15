add_subdirectory(core)
add_subdirectory(native)

option(YDB_CPP_SDK_BUILD_SLO_USERVER "Build userver SLO workload (requires userver)" OFF)
if (YDB_CPP_SDK_BUILD_SLO_USERVER)
  add_subdirectory(userver)
endif()
