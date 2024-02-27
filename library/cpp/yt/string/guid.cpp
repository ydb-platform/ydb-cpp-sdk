#include "guid.h"

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

void FormatValue(TStringBuilderBase* builder, TGuid value, std::string_view /*format*/)
{
    char* begin = builder->Preallocate(MaxGuidStringSize);
    char* end = WriteGuidToBuffer(begin, value);
    builder->Advance(end - begin);
}

std::string ToString(TGuid guid)
{
    return ToStringViaBuilder(guid);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT

