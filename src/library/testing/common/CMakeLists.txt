add_library(cpp-testing-common)

target_link_libraries(cpp-testing-common PUBLIC
  yutil
  library-json
  library-svnversion
)

target_sources(cpp-testing-common PRIVATE
  env.cpp
  env_var.cpp
  network.cpp
  probe.cpp
  scope.cpp
)

vcs_info(cpp-testing-common)
