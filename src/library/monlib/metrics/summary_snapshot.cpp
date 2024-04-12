#include "summary_snapshot.h"

#include <src/util/stream/output.h>

#include <iostream>


namespace {

template <typename TStream>
auto& Output(TStream& o, const NMonitoring::ISummaryDoubleSnapshot& s) {
    o << std::string_view("{");

    o << std::string_view("sum: ") << s.GetSum() << std::string_view(", ");
    o << std::string_view("min: ") << s.GetMin() << std::string_view(", ");
    o << std::string_view("max: ") << s.GetMax() << std::string_view(", ");
    o << std::string_view("last: ") << s.GetLast() << std::string_view(", ");
    o << std::string_view("count: ") << s.GetCount();

    o << std::string_view("}");

    return o;
}

} // namespace

std::ostream& operator<<(std::ostream& o, const NMonitoring::ISummaryDoubleSnapshot& s) {
    return Output(o, s);
}

template <>
void Out<NMonitoring::ISummaryDoubleSnapshot>(IOutputStream& o, const NMonitoring::ISummaryDoubleSnapshot& s) {
    Output(o, s);
}
