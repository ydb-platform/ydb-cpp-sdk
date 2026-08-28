add_executable(slo-topic)

target_sources(slo-topic PRIVATE
  main.cpp
  native.cpp
  topic.cpp
)

target_link_libraries(slo-topic PRIVATE
  slo-utils
  YDB-CPP-SDK::Topic
)

vcs_info(slo-topic)
