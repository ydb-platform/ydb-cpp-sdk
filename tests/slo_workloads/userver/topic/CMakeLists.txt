add_executable(slo-userver-topic)

target_sources(slo-userver-topic PRIVATE
  main.cpp
  topic.cpp
  ${CMAKE_SOURCE_DIR}/tests/slo_workloads/topic/topic.cpp
  ${CMAKE_SOURCE_DIR}/tests/slo_workloads/userver/key_value/userver_table_client.cpp
  ${CMAKE_SOURCE_DIR}/tests/slo_workloads/userver/key_value/userver_utils.cpp
)

target_link_libraries(slo-userver-topic PRIVATE
  slo-utils-base
  userver::ydb
  userver::core
  YDB-CPP-SDK::Topic
)

if(DEFINED USERVER_ROOT_DIR)
  target_include_directories(slo-userver-topic PRIVATE "${USERVER_ROOT_DIR}/ydb/src")
elseif(DEFINED ENV{USERVER_SOURCE_DIR})
  target_include_directories(slo-userver-topic PRIVATE "$ENV{USERVER_SOURCE_DIR}/ydb/src")
endif()

vcs_info(slo-userver-topic)
