add_library(cpp-testing-common)

target_link_libraries(cpp-testing-common PUBLIC
  yutil
  json
  svnversion
)

target_sources(cpp-testing-common PRIVATE
  env.cpp
  network.cpp
  probe.cpp
  scope.cpp
)

vcs_info(cpp-testing-common)
