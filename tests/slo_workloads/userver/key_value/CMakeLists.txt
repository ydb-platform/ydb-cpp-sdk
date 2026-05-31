add_executable(slo-userver-key-value)

target_sources(slo-userver-key-value PRIVATE
  main.cpp
  userver_utils.cpp
  userver_table_client.cpp
  key_value.cpp
  create.cpp
  run.cpp
  # CreateTable / DropTable: reuse native sources verbatim.
  ${CMAKE_SOURCE_DIR}/tests/slo_workloads/key_value/create.cpp
  ${CMAKE_SOURCE_DIR}/tests/slo_workloads/key_value/drop.cpp
)

target_link_libraries(slo-userver-key-value PRIVATE
  slo-utils-base       # Shared utils: ParseOptions*, GetTableStats, YdbStatusToString, etc.
  userver::ydb         # ydb::TableClient (workload queries)
  userver::core        # engine::RunStandalone, Semaphore, AsyncNoSpan
)

# userver::ydb impl headers (ydb/impl/driver.hpp) live under the source tree, not in the .deb.
if(DEFINED USERVER_ROOT_DIR)
  target_include_directories(slo-userver-key-value PRIVATE "${USERVER_ROOT_DIR}/ydb/src")
elseif(DEFINED ENV{USERVER_SOURCE_DIR})
  target_include_directories(slo-userver-key-value PRIVATE "$ENV{USERVER_SOURCE_DIR}/ydb/src")
endif()

vcs_info(slo-userver-key-value)
