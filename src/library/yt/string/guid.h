<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/fwd.h>
=======
#include <src/util/generic/fwd.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

#include <src/library/yt/misc/guid.h>

#include "format.h"

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

void FormatValue(TStringBuilderBase* builder, TGuid value, std::string_view /*format*/);
std::string ToString(TGuid guid);

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
