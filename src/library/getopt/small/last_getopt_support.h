#pragma once

#include <src/library/string_utils/misc/misc.h>

#include <src/util/string/cast.h>
#include <src/util/string/escape.h>

#include <src/util/generic/utility.h>
#include <src/util/generic/yexception.h>

#include <string>

namespace NLastGetopt {
    class TOpt;
    class TOpts;
    class TOptsParser;
    class TOptsParseResult;

    /// base of all getopt exceptions
    class TException: public yexception {
    };

    /// TOpts configuration is incorrect
    class TConfException: public TException {
    };

    /// User passed incorrect arguments, parsing failed
    /// Note: use `throw TUsageException()` instead of `ythrow TUsageException()` to prevent appearence of stacktrace
    /// and location of the `ythrow` statment in error messages.
    class TUsageException: public TException {
    };

    struct IOptHandler {
        virtual void HandleOpt(const TOptsParser* parser) = 0;
        virtual ~IOptHandler() = default;
    };

    namespace NPrivate {
        template <typename TpFunc>
        class THandlerFunctor0
           : public IOptHandler {
            TpFunc Func_;

        public:
            THandlerFunctor0(TpFunc func)
                : Func_(func)
            {
            }

            void HandleOpt(const TOptsParser*) override {
                Func_();
            }
        };

        template <typename TpFunc, typename TpArg = const TOptsParser*>
        class THandlerFunctor1
           : public IOptHandler {
            TpFunc Func_;
            const TpArg Def_;
            const bool HasDef_;

        public:
            THandlerFunctor1(TpFunc func)
                : Func_(func)
                , Def_()
                , HasDef_(false)
            {
            }

            template <typename T>
            THandlerFunctor1(const TpFunc& func, const T& def)
                : Func_(func)
                , Def_(def)
                , HasDef_(true)
            {
            }

            void HandleOpt(const TOptsParser* parser) override;
        };

        template <typename TpFunc>
        class THandlerFunctor1<TpFunc, const TOptsParser*>
           : public IOptHandler {
            TpFunc Func_;

        public:
            THandlerFunctor1(TpFunc func)
                : Func_(func)
            {
            }

            void HandleOpt(const TOptsParser* parser) override {
                Func_(parser);
            }
        };

        template <typename T, typename TpVal = T>
        class TStoreResultFunctor {
        private:
            T* Target_;

        public:
            TStoreResultFunctor(T* target)
                : Target_(target)
            {
            }

            void operator()(const TpVal& val) {
                *Target_ = val;
            }
        };

        template <typename TpTarget, typename TpFunc, typename TpVal = TpTarget>
        class TStoreMappedResultFunctor {
        private:
            TpTarget* Target_;
            const TpFunc Func_;

        public:
            TStoreMappedResultFunctor(TpTarget* target, const TpFunc& func)
                : Target_(target)
                , Func_(func)
            {
            }

            void operator()(const TpVal& val) {
                *Target_ = Func_(val);
            }
        };

        template <typename T, typename TpVal = T>
        class TStoreValueFunctor {
            T* Target;
            const TpVal Value;

        public:
            template <typename TpArg>
            TStoreValueFunctor(T* target, const TpArg& value)
                : Target(target)
                , Value(value)
            {
            }

            void operator()(const TOptsParser*) {
                *Target = Value;
            }
        };

        std::string OptToString(char c);
        std::string OptToString(const std::string& longOption);
        std::string OptToString(const TOpt* opt);

        template <typename T>
        inline T OptFromStringImpl(const std::string_view& value) {
            return FromString<T>(value);
        }

        template <>
        inline std::string_view OptFromStringImpl<std::string_view>(const std::string_view& value) {
            return value;
        }

        template <>
        inline const char* OptFromStringImpl<const char*>(const std::string_view& value) {
            return value.data();
        }

        template <typename T, typename TSomeOpt>
        T OptFromString(const std::string_view& value, const TSomeOpt opt) {
            try {
                return OptFromStringImpl<T>(value);
            } catch (...) {
                throw TUsageException() << "failed to parse opt " << OptToString(opt) << " value " << NUtils::Quote(value) << ": " << CurrentExceptionMessage();
            }
        }

        // wrapper of FromString<T> that prints nice message about option used
        template <typename T, typename TSomeOpt>
        T OptFromString(const std::string_view& value, const TSomeOpt opt);

    }
}
