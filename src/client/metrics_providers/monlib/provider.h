#pragma once

#include <src/client/impl/internal/metrics/metrics.h>

#include <library/cpp/monlib/metrics/metric_registry.h>

namespace NYdb::inline V3::NMetrics {

std::shared_ptr<NMetrics::IMetricsProvider> CreateMonlibMetricsProvider(::NMonitoring::IMetricRegistry* registry);

::NMonitoring::IMetricRegistry* TryGetUnderlyingMetricsRegistry(std::shared_ptr<NMetrics::IMetricsProvider> metricsProvider);

}
