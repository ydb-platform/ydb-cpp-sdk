#include "memory_tag.h"

#include <src/library/yt/misc/tls.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/compiler.h>
=======
#include <src/util/system/compiler.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/compiler.h>
>>>>>>> origin/main

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

YT_THREAD_LOCAL(TMemoryTag) CurrentMemoryTag;

Y_WEAK TMemoryTag GetCurrentMemoryTag()
{
    return CurrentMemoryTag;
}

Y_WEAK void SetCurrentMemoryTag(TMemoryTag tag)
{
    CurrentMemoryTag = tag;
}

Y_WEAK size_t GetMemoryUsageForTag(TMemoryTag /*tag*/)
{
    return 0;
}

Y_WEAK void GetMemoryUsageForTags(const TMemoryTag* /*tags*/, size_t /*count*/, size_t* /*results*/)
{ }

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT

