#pragma once

#include <string_view>
<<<<<<< HEAD
<<<<<<<< HEAD:include/ydb-cpp-sdk/library/json/common/defs.h
#include <ydb-cpp-sdk/util/generic/yexception.h>
========
#include <src/util/generic/yexception.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/json/common/defs.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/library/json/common/defs.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/yexception.h>
>>>>>>> origin/main

namespace NJson {
    class TJsonException: public yexception {
    };

    class TJsonCallbacks {
    public:
        explicit TJsonCallbacks(bool throwException = false)
            : ThrowException(throwException)
        {
        }

        virtual ~TJsonCallbacks();

        virtual bool OnNull();
        virtual bool OnBoolean(bool);
        virtual bool OnInteger(long long);
        virtual bool OnUInteger(unsigned long long);
        virtual bool OnDouble(double);
        virtual bool OnString(const std::string_view&);
        virtual bool OnOpenMap();
        virtual bool OnMapKey(const std::string_view&);
        virtual bool OnCloseMap();
        virtual bool OnOpenArray();
        virtual bool OnCloseArray();
        virtual bool OnStringNoCopy(const std::string_view& s);
        virtual bool OnMapKeyNoCopy(const std::string_view& s);
        virtual bool OnEnd();
        virtual void OnError(size_t off, std::string_view reason);

        bool GetHaveErrors() const {
            return HaveErrors;
        }
    protected:
        bool ThrowException;
        bool HaveErrors = false;
    };
}
