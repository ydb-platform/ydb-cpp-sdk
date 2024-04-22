#include "output.h"

#include <ydb-cpp-sdk/util/stream/output.h>

using namespace NColorizer;

template <>
void Out<TColorHandle>(IOutputStream& o, const TColorHandle& h) {
    o << (*(h.C).*h.F)();
}


std::ostream& operator<<(std::ostream& o, const TColorHandle& h) {
    return o << (*(h.C).*h.F)();
}
