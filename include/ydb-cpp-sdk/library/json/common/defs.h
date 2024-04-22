#pragma once

#include <string_view>
#include <ydb-cpp-sdk/util/generic/yexception.h>

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
