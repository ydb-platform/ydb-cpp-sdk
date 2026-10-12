if(NOT PROJECT_NAME STREQUAL "YDB-CPP-SDK")
  return()
endif()

execute_process(
  COMMAND nm -g "${CMAKE_BINARY_DIR}/tests/slo_workloads/userver/key_value/slo-userver-key-value"
  OUTPUT_VARIABLE _symbols
  COMMAND_ERROR_IS_FATAL ANY
)
string(REGEX MATCH "_ZN[^ \n]*11TableClient16ExecuteDataQuery[^ \n]*" _query "${_symbols}")
string(REGEX MATCH "_Z13GetTableStats[^ \n]*" _stats "${_symbols}")
string(REGEX MATCH "_Z11CreateTable[^ \n]*" _create "${_symbols}")
if(NOT _query OR NOT _stats OR NOT _create)
  message(FATAL_ERROR "Actual userver SDK boundary symbols not found")
endif()

add_executable(cpp-final-error-userver-probe
  /probe/userver.cpp
  "${CMAKE_BINARY_DIR}/tests/slo_workloads/userver/key_value/__vcs_version__.c"
  tests/slo_workloads/userver/key_value/userver_table_client.cpp
  tests/slo_workloads/userver/key_value/key_value.cpp
  tests/slo_workloads/userver/key_value/create.cpp
  tests/slo_workloads/userver/key_value/run.cpp
  tests/slo_workloads/key_value/create.cpp
  tests/slo_workloads/key_value/drop.cpp
)
target_include_directories(cpp-final-error-userver-probe PRIVATE
  "${CMAKE_SOURCE_DIR}" "$ENV{USERVER_SOURCE_DIR}/ydb/src")
target_compile_features(cpp-final-error-userver-probe PRIVATE cxx_std_20)
target_link_libraries(cpp-final-error-userver-probe PRIVATE
  slo-utils-base userver::ydb userver::core)
target_compile_definitions(cpp-final-error-userver-probe PRIVATE
  QUERY_WRAP_SYMBOL="__wrap_${_query}"
  STATS_WRAP_SYMBOL="__wrap_${_stats}"
  CREATE_WRAP_SYMBOL="__wrap_${_create}"
)
target_link_options(cpp-final-error-userver-probe PRIVATE
  "-Wl,--wrap=${_query}" "-Wl,--wrap=${_stats}" "-Wl,--wrap=${_create}")
