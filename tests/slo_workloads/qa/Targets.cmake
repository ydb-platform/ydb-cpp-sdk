if(NOT PROJECT_NAME STREQUAL "YDB-CPP-SDK")
  return()
endif()

execute_process(
  COMMAND nm -g "${CMAKE_BINARY_DIR}/tests/slo_workloads/utils/libslo-utils-base.a"
  OUTPUT_VARIABLE _symbols
  COMMAND_ERROR_IS_FATAL ANY
)
string(REGEX MATCH "_ZN[^ \n]*12TTableClient14RetryOperation[^ \n]*" _operation "${_symbols}")
string(REGEX MATCH "_Z13GetTableStats[^ \n]*" _stats "${_symbols}")

execute_process(
  COMMAND nm -g "${CMAKE_BINARY_DIR}/tests/slo_workloads/key_value/CMakeFiles/slo-key-value.dir/create.cpp.o"
  OUTPUT_VARIABLE _create_symbols
  COMMAND_ERROR_IS_FATAL ANY
)
string(REGEX MATCH "_Z11CreateTable[^ \n]*" _create "${_create_symbols}")
if(NOT _operation OR NOT _stats OR NOT _create)
  message(FATAL_ERROR "Actual SDK boundary symbols not found")
endif()

add_executable(cpp-final-error-probe
  /probe/native.cpp
  "${CMAKE_BINARY_DIR}/tests/slo_workloads/key_value/__vcs_version__.c"
  tests/slo_workloads/key_value/create.cpp
  tests/slo_workloads/key_value/drop.cpp
  tests/slo_workloads/key_value/generate.cpp
  tests/slo_workloads/key_value/key_value.cpp
  tests/slo_workloads/key_value/run.cpp
)
target_include_directories(cpp-final-error-probe PRIVATE "${CMAKE_SOURCE_DIR}")
target_compile_features(cpp-final-error-probe PRIVATE cxx_std_20)
target_link_libraries(cpp-final-error-probe PRIVATE slo-utils YDB-CPP-SDK::Table)
target_compile_definitions(cpp-final-error-probe PRIVATE
  OPERATION_WRAP_SYMBOL="__wrap_${_operation}"
  STATS_WRAP_SYMBOL="__wrap_${_stats}"
  CREATE_WRAP_SYMBOL="__wrap_${_create}"
)
target_link_options(cpp-final-error-probe PRIVATE
  "-Wl,--wrap=${_operation}"
  "-Wl,--wrap=${_stats}"
  "-Wl,--wrap=${_create}"
  -ldl -lrt -Wl,--no-as-needed -lpthread
)
