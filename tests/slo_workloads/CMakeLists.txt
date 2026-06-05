add_subdirectory(key_value)
add_subdirectory(utils)

option(YDB_CPP_SDK_BUILD_SLO_USERVER "Build userver SLO workload (requires userver)" OFF)
if (YDB_CPP_SDK_BUILD_SLO_USERVER)
  add_subdirectory(userver)
endif()
