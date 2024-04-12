#include "tempfile.h"

#include <src/library/testing/unittest/registar.h>

#include <util/folder/dirut.h>
#include <util/generic/yexception.h>
#include <util/stream/file.h>

#include <algorithm>

Y_UNIT_TEST_SUITE(TTempFileHandle) {
    Y_UNIT_TEST(Create) {
        std::string path;
        {
            TTempFileHandle tmp;
            path = tmp.Name();
            tmp.Write("hello world\n", 12);
            tmp.FlushData();
            UNIT_ASSERT_STRINGS_EQUAL(TUnbufferedFileInput(tmp.Name()).ReadAll(), "hello world\n");
        }
        UNIT_ASSERT(!NFs::Exists(path));
    }

    Y_UNIT_TEST(InCurrentDir) {
#ifndef _win32_
        static const std::string TEST_PREFIX = "unique_prefix";
#else
        static const std::string TEST_PREFIX = "uni";
#endif

        std::string path;
        {
            TTempFileHandle tmp = TTempFileHandle::InCurrentDir(TEST_PREFIX);
            path = tmp.Name();
            UNIT_ASSERT(NFs::Exists(path));

            std::vector<std::string> names;
            TFsPath(".").ListNames(names);
            bool containsFileWithPrefix = std::any_of(names.begin(), names.end(), [&](const std::string& name) {
                return name.Contains(TEST_PREFIX);
            });
            UNIT_ASSERT(containsFileWithPrefix);
        }
        UNIT_ASSERT(!NFs::Exists(path));
    }

    Y_UNIT_TEST(UseExtensionWithoutDot) {
        std::string path;
        {
            TTempFileHandle tmp = TTempFileHandle::InCurrentDir("hello", "world");
            path = tmp.Name();
            UNIT_ASSERT(NFs::Exists(path));

#ifndef _win32_
            UNIT_ASSERT(path.Contains("hello"));
            UNIT_ASSERT(path.EndsWith(".world"));
            UNIT_ASSERT(!path.EndsWith("..world"));
#else
            UNIT_ASSERT(path.Contains("hel"));
            UNIT_ASSERT(path.EndsWith(".tmp"));
#endif
        }
        UNIT_ASSERT(!NFs::Exists(path));
    }

    Y_UNIT_TEST(UseExtensionWithDot) {
        std::string path;
        {
            TTempFileHandle tmp = TTempFileHandle::InCurrentDir("lorem", ".ipsum");
            path = tmp.Name();
            UNIT_ASSERT(NFs::Exists(path));

#ifndef _win32_
            UNIT_ASSERT(path.Contains("lorem"));
            UNIT_ASSERT(path.EndsWith(".ipsum"));
            UNIT_ASSERT(!path.EndsWith("..ipsum"));
#else
            UNIT_ASSERT(path.Contains("lor"));
            UNIT_ASSERT(path.EndsWith(".tmp"));
#endif
        }
        UNIT_ASSERT(!NFs::Exists(path));
    }

    Y_UNIT_TEST(SafeDestructor) {
        std::string path;
        {
            path = MakeTempName();
            UNIT_ASSERT(NFs::Exists(path));

            TTempFileHandle tmp(path);
            Y_UNUSED(tmp);
            UNIT_ASSERT(NFs::Exists(path));

            TTempFileHandle anotherTmp(path);
            Y_UNUSED(anotherTmp);
            UNIT_ASSERT(NFs::Exists(path));
        }
        UNIT_ASSERT(!NFs::Exists(path));
    }

    Y_UNIT_TEST(RemovesOpen) {
        std::string path;
        {
            TTempFileHandle tmp;
            path = tmp.Name();
            tmp.Write("hello world\n", 12);
            tmp.FlushData();
            UNIT_ASSERT(NFs::Exists(path));
            UNIT_ASSERT(tmp.IsOpen());
        }
        UNIT_ASSERT(!NFs::Exists(path));
    }

    Y_UNIT_TEST(NonExistingDirectory) {
        UNIT_ASSERT_EXCEPTION(TTempFileHandle::InDir("nonexsistingdirname"), TSystemError);
    }
}

Y_UNIT_TEST_SUITE(MakeTempName) {
    Y_UNIT_TEST(Default) {
        std::string path;
        {
            TTempFile tmp(MakeTempName());
            path = tmp.Name();

            UNIT_ASSERT(!path.Contains('\0'));
            UNIT_ASSERT(NFs::Exists(path));
            UNIT_ASSERT(path.EndsWith(".tmp"));

#ifndef _win32_
            UNIT_ASSERT(path.Contains("yandex"));
#else
            UNIT_ASSERT(path.Contains("yan"));
#endif
        }
        UNIT_ASSERT(!NFs::Exists(path));
    }

    Y_UNIT_TEST(UseNullptr) {
        std::string path;
        {
            TTempFile tmp(MakeTempName(nullptr, nullptr, nullptr));
            path = tmp.Name();

            UNIT_ASSERT(!path.Contains('\0'));
            UNIT_ASSERT(NFs::Exists(path));
        }
        UNIT_ASSERT(!NFs::Exists(path));
    }
}
