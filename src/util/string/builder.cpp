#include <ydb-cpp-sdk/util/string/builder.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/output.h>
=======
#include <src/util/stream/output.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

template <>
void Out<TStringBuilder>(IOutputStream& os, const TStringBuilder& sb) {
    os << static_cast<const std::string&>(sb);
}
