# Base utils library: shared between native and userver workloads
# Contains: ParseOptions*, GetTableStats, YdbStatusToString, PackValuesToParamsAsList, etc.
# Does NOT contain DoMain (each workload provides its own).
add_library(slo-utils-base)

target_link_libraries(slo-utils-base PUBLIC
  slo-workload-core
  yutil
  getopt
  YDB-CPP-SDK::Table
  YDB-CPP-SDK::Iam
)

if (SLO_BRANCH_REF)
  target_compile_definitions(slo-utils-base PRIVATE REF=${SLO_BRANCH_REF})
endif()

target_sources(slo-utils-base PRIVATE
  utils.cpp
)

# Full native utils library: adds DoMain, executor and job infrastructure (thread-based)
add_library(slo-utils)

target_link_libraries(slo-utils PUBLIC
  slo-utils-base
)

target_sources(slo-utils PRIVATE
  utils_main.cpp
  executor.cpp
  job.cpp
)
