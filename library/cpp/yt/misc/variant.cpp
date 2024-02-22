#include "variant.h"

#include <library/cpp/yt/string/string_builder.h>

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

void FormatValue(TStringBuilderBase* builder, const std::monostate&, std::string_view /*format*/)
{
    builder->AppendString("<monostate>");
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
