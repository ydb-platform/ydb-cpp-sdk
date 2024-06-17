add_library(library-cpuid_check INTERFACE)

target_link_libraries(library-cpuid_check INTERFACE
  yutil
)

add_global_library_for(library-cpuid_check.global library-cpuid_check)

target_link_libraries(library-cpuid_check.global PUBLIC
  yutil
)

target_sources(library-cpuid_check.global PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/cpuid_check/cpu_id_check.cpp
)

_ydb_sdk_install_targets(TARGETS library-cpuid_check)
_ydb_sdk_install_targets(TARGETS library-cpuid_check.global.wholearchive)
