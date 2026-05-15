add_library(slo-utils)

target_link_libraries(slo-utils PUBLIC
  slo-workload-core
  yutil
  getopt
  YDB-CPP-SDK::Table
  YDB-CPP-SDK::Iam
)

if (SLO_BRANCH_REF)
  target_compile_definitions(slo-utils PRIVATE REF=${SLO_BRANCH_REF})
endif()

target_sources(slo-utils PRIVATE
  executor.cpp
  job.cpp
  utils.cpp
)
