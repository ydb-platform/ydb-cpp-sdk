#include <src/library/testing/unittest/registar.h>

#include "filemap.h"
#include "info.h"
#include "mincore.h"
#include "mlock.h"
#include "tempfile.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/size_literals.h>
=======
#include <src/util/generic/size_literals.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

Y_UNIT_TEST_SUITE(MincoreSuite) {
    static const char* FileName_("./mappped_file");

    Y_UNIT_TEST(TestLockAndInCore) {
        std::vector<char> content(2_MB);

        TTempFile cleanup(FileName_);
        TFile file(FileName_, CreateAlways | WrOnly);
        file.Write(content.data(), content.size());
        file.Close();

        TFileMap mappedFile(FileName_, TMemoryMapCommon::oRdWr);
        mappedFile.Map(0, mappedFile.Length());
        UNIT_ASSERT_EQUAL(mappedFile.MappedSize(), content.size());
        UNIT_ASSERT_EQUAL(mappedFile.Length(), static_cast<i64>(content.size()));

        LockMemory(mappedFile.Ptr(), mappedFile.Length());

        std::vector<unsigned char> incore(mappedFile.Length() / NSystemInfo::GetPageSize());
        InCoreMemory(mappedFile.Ptr(), mappedFile.Length(), incore.data(), incore.size());

        // compile and run on all platforms, but assume non-zero results only on Linux
#if defined(_linux_)
        for (const auto& flag : incore) {
            UNIT_ASSERT(IsPageInCore(flag));
        }
#endif
    }
}
