#pragma once

<<<<<<< HEAD
=======
<<<<<<<< HEAD:src/library/monlib/metrics/log_histogram_snapshot.h
>>>>>>> origin/main
<<<<<<<< HEAD:include/ydb-cpp-sdk/library/monlib/metrics/log_histogram_snapshot.h
#include <ydb-cpp-sdk/util/generic/ptr.h>
========
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/monlib/metrics/log_histogram_snapshot.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/library/monlib/metrics/log_histogram_snapshot.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD
=======
========
#include <ydb-cpp-sdk/util/generic/ptr.h>
>>>>>>>> origin/main:include/ydb-cpp-sdk/library/monlib/metrics/log_histogram_snapshot.h
>>>>>>> origin/main

#include <vector>
#include <cmath>

namespace NMonitoring {

    constexpr ui32 LOG_HIST_MAX_BUCKETS = 100;

    class TLogHistogramSnapshot: public TAtomicRefCount<TLogHistogramSnapshot> {
    public:
        TLogHistogramSnapshot(double base, ui64 zerosCount, int startPower, std::vector<double> buckets)
            : Base_(base)
            , ZerosCount_(zerosCount)
            , StartPower_(startPower)
            , Buckets_(std::move(buckets)) {
        }

        /**
         * @return buckets count.
         */
        ui32 Count() const noexcept {
            return Buckets_.size();
        }

        /**
         * @return upper bound for the bucket with particular index.
         */
        double UpperBound(int index) const noexcept {
            return std::pow(Base_, StartPower_ + index);
        }

        /**
         * @return value stored in the bucket with particular index.
         */
        double Bucket(ui32 index) const noexcept {
            return Buckets_[index];
        }

        /**
         * @return nonpositive values count
         */
        ui64 ZerosCount() const noexcept {
            return ZerosCount_;
        }

        double Base() const noexcept {
            return Base_;
        }

        int StartPower() const noexcept {
            return StartPower_;
        }

        ui64 MemorySizeBytes() const noexcept {
            return sizeof(*this) + Buckets_.capacity() * sizeof(double);
        }

    private:
        double Base_;
        ui64 ZerosCount_;
        int StartPower_;
        std::vector<double> Buckets_;
    };

    using TLogHistogramSnapshotPtr = TIntrusivePtr<TLogHistogramSnapshot>;
}

std::ostream& operator<<(std::ostream& os, const NMonitoring::TLogHistogramSnapshot& hist);
