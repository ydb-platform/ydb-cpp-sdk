#pragma once

#include "userver_utils.h"

#include <tests/slo_workloads/utils/generator.h>
#include <tests/slo_workloads/utils/statistics.h>

#include <ydb-cpp-sdk/client/value/value.h>

#include <string>

extern const std::string TableName;

NYdb::TValue BuildValueFromRecord(const TKeyValueRecordData& recordData);

int CreateTable(TDatabaseOptions& dbOptions);
int DropTable(TDatabaseOptions& dbOptions);

// Creates a table and fills it with initial content (userver coroutine-based)
int DoCreate(TDatabaseOptions& dbOptions, int argc, char** argv);
// Runs read/write workload (userver coroutine-based)
int DoRun(TDatabaseOptions& dbOptions, int argc, char** argv);
// Drops the table
int DoCleanup(TDatabaseOptions& dbOptions, int argc);
