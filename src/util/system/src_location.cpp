#include <ydb-cpp-sdk/util/system/src_location.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/output.h>
=======
#include <src/util/stream/output.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

template <>
void Out<TSourceLocation>(IOutputStream& o, const TSourceLocation& t) {
#if defined(_win_)
    std::string file(t.File);
    std::replace(file.begin(), file.vend(), '\\', '/');
    o << file;
#else
    o << t.File;
#endif
    o << ':' << t.Line;
}
