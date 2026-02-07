#include <ydb-cpp-sdk/client/metrics/otel/otel.h>

#include <opentelemetry/metrics/meter.h>
#include <opentelemetry/trace/tracer.h>

namespace NYdb::inline V3::NMetrics {

namespace {

using namespace opentelemetry;

common::KeyValueIteratable<TLabels> MakeAttributes(const TLabels& labels) {
    return common::KeyValueIteratable<TLabels>(labels);
}

class TOtelCounter : public ICounter {
public:
    TOtelCounter(nostd::shared_ptr<metrics::Counter<uint64_t>> counter, const TLabels& labels)
        : Counter_(std::move(counter))
        , Labels_(labels)
    {}

    void Inc() override {
        Counter_->Add(1, MakeAttributes(Labels_));
    }

private:
    nostd::shared_ptr<metrics::Counter<uint64_t>> Counter_;
    TLabels Labels_;
};

class TOtelGauge : public IGauge {
public:
    TOtelGauge(nostd::shared_ptr<metrics::ObservableGauge<double>> gauge, const TLabels& labels)
        : Gauge_(std::move(gauge))
        , Labels_(labels)
    {}

    void Add(double delta) override {
        // TODO
    }

    void Set(double value) override {
        Value_ = value;
    }

private:
    nostd::shared_ptr<metrics::ObservableGauge<double>> Gauge_;
    TLabels Labels_;
    double Value_ = 0;
};

class TOtelUpDownCounterGauge : public IGauge {
public:
    TOtelUpDownCounterGauge(nostd::shared_ptr<metrics::UpDownCounter<double>> counter, const TLabels& labels)
        : Counter_(std::move(counter))
        , Labels_(labels)
    {}

    void Add(double delta) override {
        Counter_->Add(delta, MakeAttributes(Labels_));
        Value_ += delta;
    }

    void Set(double value) override {
        Counter_->Add(value - Value_, MakeAttributes(Labels_));
        Value_ = value;
    }

private:
    nostd::shared_ptr<metrics::UpDownCounter<double>> Counter_;
    TLabels Labels_;
    double Value_ = 0;
};

class TOtelHistogram : public IHistogram {
public:
    TOtelHistogram(nostd::shared_ptr<metrics::Histogram<double>> histogram, const TLabels& labels)
        : Histogram_(std::move(histogram))
        , Labels_(labels)
    {}

    void Record(double value) override {
        Histogram_->Record(value, MakeAttributes(Labels_));
    }

private:
    nostd::shared_ptr<metrics::Histogram<double>> Histogram_;
    TLabels Labels_;
};

trace::SpanKind MapSpanKind(ESpanKind kind) {
    switch (kind) {
        case ESpanKind::INTERNAL: return trace::SpanKind::kInternal;
        case ESpanKind::SERVER:   return trace::SpanKind::kServer;
        case ESpanKind::CLIENT:   return trace::SpanKind::kClient;
        case ESpanKind::PRODUCER: return trace::SpanKind::kProducer;
        case ESpanKind::CONSUMER: return trace::SpanKind::kConsumer;
    }
    return trace::SpanKind::kInternal;
}

class TOtelSpan : public ISpan {
public:
    TOtelSpan(nostd::shared_ptr<trace::Span> span)
        : Span_(std::move(span))
    {}

    void End() override {
        Span_->End();
    }

    void SetAttribute(const std::string& key, const std::string& value) override {
        Span_->SetAttribute(key, value);
    }

    void SetAttribute(const std::string& key, int64_t value) override {
        Span_->SetAttribute(key, value);
    }

    void SetAttribute(const std::string& key, double value) override {
        Span_->SetAttribute(key, value);
    }

    void SetAttribute(const std::string& key, bool value) override {
        Span_->SetAttribute(key, value);
    }

private:
    nostd::shared_ptr<trace::Span> Span_;
};

class TOtelTracer : public ITracer {
public:
    TOtelTracer(nostd::shared_ptr<trace::Tracer> tracer)
        : Tracer_(std::move(tracer))
    {}

    std::shared_ptr<ISpan> StartSpan(const std::string& name, ESpanKind kind) override {
        trace::StartSpanOptions options;
        options.kind = MapSpanKind(kind);
        return std::make_shared<TOtelSpan>(Tracer_->StartSpan(name, options));
    }

private:
    nostd::shared_ptr<trace::Tracer> Tracer_;
};

} // namespace

TOtelMetricRegistry::TOtelMetricRegistry(nostd::shared_ptr<metrics::MeterProvider> meterProvider)
    : MeterProvider_(std::move(meterProvider))
    , Meter_(MeterProvider_->GetMeter("ydb-cpp-sdk", "1.0.0"))
{}

std::shared_ptr<ICounter> TOtelMetricRegistry::Counter(const std::string& name, const TLabels& labels) {
    auto counter = Meter_->CreateUInt64Counter(name);
    return std::make_shared<TOtelCounter>(std::move(counter), labels);
}

std::shared_ptr<IGauge> TOtelMetricRegistry::Gauge(const std::string& name, const TLabels& labels) {
    auto counter = Meter_->CreateDoubleUpDownCounter(name);
    return std::make_shared<TOtelUpDownCounterGauge>(std::move(counter), labels);
}

std::shared_ptr<IHistogram> TOtelMetricRegistry::Histogram(const std::string& name, const std::vector<double>& buckets, const TLabels& labels) {
    auto histogram = Meter_->CreateDoubleHistogram(name);
    return std::make_shared<TOtelHistogram>(std::move(histogram), labels);
}

TOtelTraceProvider::TOtelTraceProvider(nostd::shared_ptr<trace::TracerProvider> tracerProvider)
    : TracerProvider_(std::move(tracerProvider))
{}

std::shared_ptr<ITracer> TOtelTraceProvider::GetTracer(const std::string& name) {
    return std::make_shared<TOtelTracer>(TracerProvider_->GetTracer(name));
}

} // namespace NYdb::NMetrics
