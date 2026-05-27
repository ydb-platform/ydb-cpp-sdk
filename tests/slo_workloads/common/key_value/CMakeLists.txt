# Shared key-value workload library: TableName, BuildValueFromRecord, CreateTable, DropTable
# Used by both native and userver SLO workloads.
add_library(slo-kv-common)

target_link_libraries(slo-kv-common PUBLIC
  slo-utils-base
)

target_sources(slo-kv-common PRIVATE
  kv_data.cpp
  create.cpp
  drop.cpp
)
