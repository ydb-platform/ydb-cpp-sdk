option(YDB_SDK_COVERAGE "Instrument SDK and tests for gcov/llvm-cov coverage" OFF)

if (YDB_SDK_COVERAGE)
  add_link_options(--coverage)
endif()

function(_ydb_sdk_apply_coverage target)
  if (NOT YDB_SDK_COVERAGE)
    return()
  endif()
  get_target_property(is_iface ${target} TYPE)
  if (is_iface STREQUAL "INTERFACE_LIBRARY")
    return()
  endif()
  # Do not use -fprofile-abs-path: it expands __FILE__/__LOCATION__ to absolute paths
  # and breaks util/system/src_location_ut and src_root_ut (and similar assertions).
  # gcovr --root maps profile data to the source tree without it.
  set(_coverage_compile_flags --coverage -O0 -g -fno-inline)
  target_compile_options(${target} PRIVATE ${_coverage_compile_flags})
  target_link_options(${target} PRIVATE --coverage)
endfunction()
