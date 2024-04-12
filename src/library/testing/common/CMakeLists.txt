add_library(cpp-testing-common)
target_link_libraries(cpp-testing-common PUBLIC
  yutil
  library-cpp-json
  library-cpp-svnversion
)
target_sources(cpp-testing-common PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/common/env.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/common/env_var.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/common/network.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/common/probe.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/common/scope.cpp
)

vcs_info(cpp-testing-common)

add_subdirectory(ut)
