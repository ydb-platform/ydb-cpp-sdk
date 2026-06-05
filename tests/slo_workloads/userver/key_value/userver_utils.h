#pragma once

// Re-export the shared utils header. The userver workload uses the same
// CLI parsing, option structs, and helper functions as the native workload.
// Only DoMain() is overridden in userver_utils.cpp to wrap command execution
// inside userver::engine::RunStandalone().
#include <tests/slo_workloads/utils/utils.h>
