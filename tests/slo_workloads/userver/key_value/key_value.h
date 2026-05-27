#pragma once

#include "userver_utils.h"

#include <tests/slo_workloads/common/key_value/kv_common.h>

#include <tests/slo_workloads/core/statistics.h>

#include <string>
#include <atomic>
#include <memory>

// Creates a table and fills it with initial content (userver coroutine-based)
int DoCreate(TDatabaseOptions& dbOptions, int argc, char** argv);
// Runs read/write workload (userver coroutine-based)
int DoRun(TDatabaseOptions& dbOptions, int argc, char** argv);
// Drops the table
int DoCleanup(TDatabaseOptions& dbOptions, int argc);
