#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NPrivate {

    /*
     * cxxabi::__cxa_demangle (and thus TCppDemanger) have terrible memory ownership model.
     *
     * Consider using CppDemangle instead. It is slow, but robust.
     */
    class TCppDemangler {
    public:
        const char* Demangle(const char* name);

    private:
        THolder<char, TFree> TmpBuf_;
    };

} //namespace NPrivate
