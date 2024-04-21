UNITTEST_FOR(src/library/monlib/metrics)

SRCS(
    ewma_ut.cpp
    fake_ut.cpp
    histogram_collector_ut.cpp
    histogram_snapshot_ut.cpp
    labels_ut.cpp
    log_histogram_collector_ut.cpp
    metric_registry_ut.cpp
    metric_sub_registry_ut.cpp
    metric_value_ut.cpp
    summary_collector_ut.cpp
    timer_ut.cpp
)

RESOURCE(
    histograms.json /histograms.json
)

PEERDIR(
    src/library/resource
    src/library/monlib/encode/protobuf
    src/library/monlib/encode/json
    src/library/threading/future
)

END()
