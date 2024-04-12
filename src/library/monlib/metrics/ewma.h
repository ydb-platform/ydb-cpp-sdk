#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/datetime/base.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/util/datetime/base.h>
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#include <atomic>

namespace NMonitoring {
    class IGauge;

    class IExpMovingAverage {
    public:
        virtual ~IExpMovingAverage() = default;
        virtual void Tick() = 0;
        virtual void Update(i64 value) = 0;
        virtual double Rate() const = 0;
        virtual void Reset() = 0;
    };

    using IExpMovingAveragePtr = THolder<IExpMovingAverage>;

    class TEwmaMeter {
    public:
        // Creates a fake EWMA that will always return 0. Mostly for usage convenience
        TEwmaMeter();
        explicit TEwmaMeter(IExpMovingAveragePtr&& ewma);

        TEwmaMeter(TEwmaMeter&& other);
        TEwmaMeter& operator=(TEwmaMeter&& other);

        void Mark();
        void Mark(i64 value);

        double Get();

    private:
        void TickIfNeeded();

    private:
        IExpMovingAveragePtr Ewma_;
        std::atomic<ui64> LastTick_{TInstant::Now().Seconds()};
    };

    IExpMovingAveragePtr OneMinuteEwma(IGauge* gauge);
    IExpMovingAveragePtr FiveMinuteEwma(IGauge* gauge);
    IExpMovingAveragePtr FiveteenMinuteEwma(IGauge* gauge);
} // namespace NMonitoring
