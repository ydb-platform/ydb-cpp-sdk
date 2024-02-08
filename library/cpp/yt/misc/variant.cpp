#include "variant.h"

#include <library/cpp/yt/string/string_builder.h>

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

void FormatValue(TYdbStringBuilderBase* builder, const std::monostate&, std::string_view /*format*/)
{
    builder->AppendString(std::string_view("<monostate>"));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
