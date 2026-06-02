option(YDB_SDK_COVERAGE "Instrument SDK and tests for gcov/llvm-cov coverage" OFF)

function(_ydb_sdk_apply_coverage target)
  if (NOT YDB_SDK_COVERAGE)
    return()
  endif()
  get_target_property(is_iface ${target} TYPE)
  if (is_iface STREQUAL "INTERFACE_LIBRARY")
    return()
  endif()
  target_compile_options(${target} PRIVATE
    --coverage
    -O0
    -g
    -fno-inline
    -fprofile-abs-path
  )
  target_link_options(${target} PRIVATE --coverage)
endfunction()
