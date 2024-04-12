#include "output.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/output.h>
=======
#include <src/util/stream/output.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

using namespace NColorizer;

template <>
void Out<TColorHandle>(IOutputStream& o, const TColorHandle& h) {
    o << (*(h.C).*h.F)();
}


std::ostream& operator<<(std::ostream& o, const TColorHandle& h) {
    return o << (*(h.C).*h.F)();
}
