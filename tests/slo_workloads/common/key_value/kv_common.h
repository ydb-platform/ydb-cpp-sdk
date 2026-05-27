#pragma once

// Shared key-value workload declarations used by both native and userver SLO tests.
// This header provides table name, value builder, and DDL operations (create/drop)
// that are identical across workload implementations.

#include <tests/slo_workloads/native/utils/utils.h>

#include <tests/slo_workloads/core/generator.h>

#include <string>

extern const std::string TableName;

NYdb::TValue BuildValueFromRecord(const TKeyValueRecordData& recordData);

int CreateTable(TDatabaseOptions& dbOptions);
int DropTable(TDatabaseOptions& dbOptions);
