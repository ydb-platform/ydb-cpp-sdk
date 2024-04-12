#include <ydb-cpp-sdk/util/string/builder.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/output.h>
=======
#include <src/util/stream/output.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

template <>
void Out<TStringBuilder>(IOutputStream& os, const TStringBuilder& sb) {
    os << static_cast<const std::string&>(sb);
}
