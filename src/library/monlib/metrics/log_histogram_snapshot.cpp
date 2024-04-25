#include <ydb-cpp-sdk/library/monlib/metrics/log_histogram_snapshot.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/output.h>
=======
#include <src/util/stream/output.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/stream/output.h>
>>>>>>> origin/main

#include <iostream>


namespace {

template <typename TStream>
auto& Output(TStream& o, const NMonitoring::TLogHistogramSnapshot& hist) {
    o << std::string_view("{");

    for (auto i = 0u; i < hist.Count(); ++i) {
        o << hist.UpperBound(i) << std::string_view(": ") << hist.Bucket(i);
        o << std::string_view(", ");
    }

    o << std::string_view("zeros: ") << hist.ZerosCount();

    o << std::string_view("}");

    return o;
}

} // namespace

std::ostream& operator<<(std::ostream& os, const NMonitoring::TLogHistogramSnapshot& hist) {
    return Output(os, hist);
}

template <>
void Out<NMonitoring::TLogHistogramSnapshot>(IOutputStream& os, const NMonitoring::TLogHistogramSnapshot& hist) {
    Output(os, hist);
}
