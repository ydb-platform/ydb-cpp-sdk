#include <util/generic/fwd.h>

#include <src/library/yt/misc/guid.h>

#include "format.h"

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

void FormatValue(TStringBuilderBase* builder, TGuid value, std::string_view /*format*/);
std::string ToString(TGuid guid);

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
