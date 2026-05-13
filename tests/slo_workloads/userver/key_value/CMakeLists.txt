add_executable(slo-userver-key-value)

target_sources(slo-userver-key-value PRIVATE
  main.cpp
)

target_link_libraries(slo-userver-key-value PRIVATE
  slo-workload-core
)

vcs_info(slo-userver-key-value)
