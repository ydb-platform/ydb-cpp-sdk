#pragma once

<<<<<<< HEAD
=======
<<<<<<<< HEAD:src/library/monlib/metrics/summary_snapshot.h
>>>>>>> origin/main
<<<<<<<< HEAD:include/ydb-cpp-sdk/library/monlib/metrics/summary_snapshot.h
#include <ydb-cpp-sdk/util/generic/ptr.h>
========
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/monlib/metrics/summary_snapshot.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/library/monlib/metrics/summary_snapshot.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD
=======
========
#include <ydb-cpp-sdk/util/generic/ptr.h>
>>>>>>>> origin/main:include/ydb-cpp-sdk/library/monlib/metrics/summary_snapshot.h
>>>>>>> origin/main

namespace NMonitoring {

    class ISummaryDoubleSnapshot: public TAtomicRefCount<ISummaryDoubleSnapshot> {
    public:
        virtual ~ISummaryDoubleSnapshot() = default;

        // TODO: write documentation

        virtual ui64 GetCount() const = 0;

        virtual double GetSum() const = 0;

        virtual double GetMin() const = 0;

        virtual double GetMax() const = 0;

        virtual double GetLast() const = 0;

        virtual ui64 MemorySizeBytes() const = 0;
    };

    using ISummaryDoubleSnapshotPtr = TIntrusivePtr<ISummaryDoubleSnapshot>;

    class TSummaryDoubleSnapshot final: public ISummaryDoubleSnapshot {
    public:
        TSummaryDoubleSnapshot(double sum, double min, double max, double last, ui64 count)
            : Sum_(sum)
            , Min_(min)
            , Max_(max)
            , Last_(last)
            , Count_(count)
        {}

        ui64 GetCount() const noexcept override {
            return Count_;
        }

        double GetSum() const noexcept override {
            return Sum_;
        }

        double GetMin() const noexcept override {
            return Min_;
        }

        double GetMax() const noexcept override {
            return Max_;
        }

        virtual double GetLast() const noexcept override {
            return Last_;
        }

        ui64 MemorySizeBytes() const noexcept override {
            return sizeof(*this);
        }

    private:
        double Sum_;
        double Min_;
        double Max_;
        double Last_;
        ui64 Count_;
    };

}

std::ostream& operator<<(std::ostream& os, const NMonitoring::ISummaryDoubleSnapshot& s);
