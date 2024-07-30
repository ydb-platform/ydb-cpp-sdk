add_library(cpuid_check INTERFACE)

add_global_library_for(cpuid_check.global cpuid_check)

target_link_libraries(cpuid_check.global PRIVATE
  yutil
)

target_sources(cpuid_check.global PRIVATE
  cpu_id_check.cpp
)

_ydb_sdk_install_targets(TARGETS cpuid_check)
_ydb_sdk_install_targets(TARGETS cpuid_check.global)
