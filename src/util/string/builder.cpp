#include <ydb-cpp-sdk/util/string/builder.h>

#include <ydb-cpp-sdk/util/stream/output.h>

template <>
void Out<TStringBuilder>(IOutputStream& os, const TStringBuilder& sb) {
    os << static_cast<const std::string&>(sb);
}
