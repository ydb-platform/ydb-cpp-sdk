#pragma once

#include "last_getopt_opts.h"
#include "last_getopt_easy_setup.h"
#include "last_getopt_parse_result.h"

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/function.h>
#include <ydb-cpp-sdk/util/string/escape.h>
=======
#include <src/util/generic/function.h>
#include <src/util/string/escape.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/function.h>
#include <ydb-cpp-sdk/util/string/escape.h>
>>>>>>> origin/main
#include <src/util/string/split.h>

/// see some documentation in
/// https://wiki.yandex-team.ru/development/poisk/arcadia/src/util/lastgetopt/
/// https://wiki.yandex-team.ru/development/poisk/arcadia/library/getopt/
/// see examples in src/library/getopt/last_getopt_demo

//TODO: in most cases this include is unnecessary, but needed THandlerFunctor1<TpFunc, TpArg>::HandleOpt
#include "last_getopt_parser.h"

namespace NLastGetopt {
    /// Handler to split option value by delimiter into a target container and allow ranges.
    template <class Container>
    struct TOptRangeSplitHandler: public IOptHandler {
    public:
        using TContainer = Container;
        using TValue = typename TContainer::value_type;

        explicit TOptRangeSplitHandler(TContainer* target, const char elementsDelim, const char rangesDelim)
            : Target(target)
            , ElementsDelim(elementsDelim)
            , RangesDelim(rangesDelim)
        {
        }

        void HandleOpt(const TOptsParser* parser) override {
            const std::string_view curval(parser->CurValOrDef());
            if (curval.data() != nullptr) {
                StringSplitter(curval).Split(ElementsDelim).Consume([&](const std::string_view& val) {
                    std::string_view mutableValue = val;

                    TValue first = NPrivate::OptFromString<TValue>(NUtils::NextTok(mutableValue, RangesDelim), parser->CurOpt());
                    TValue last = !mutableValue.empty() ? NPrivate::OptFromString<TValue>(mutableValue, parser->CurOpt()) : first;

                    if (last < first) {
                        throw TUsageException() << "failed to parse opt " << NPrivate::OptToString(parser->CurOpt()) << " value " << NUtils::Quote(val) << ": the second argument is less than the first one";
                    }

                    for (++last; first < last; ++first) {
                        Target->insert(Target->end(), first);
                    }
                });
            }
        }

    private:
        TContainer* Target;
        char ElementsDelim;
        char RangesDelim;
    };

    template <class Container>
    struct TOptSplitHandler: public IOptHandler {
    public:
        using TContainer = Container;
        using TValue = typename TContainer::value_type;

        explicit TOptSplitHandler(TContainer* target, const char delim)
            : Target(target)
            , Delim(delim)
        {
        }

        void HandleOpt(const TOptsParser* parser) override {
            const std::string_view curval(parser->CurValOrDef());
            if (curval.data() != nullptr) {
                StringSplitter(curval).Split(Delim).Consume([&](const std::string_view& val) {
                    Target->insert(Target->end(), NPrivate::OptFromString<TValue>(val, parser->CurOpt()));
                });
            }
        }

    private:
        TContainer* Target;
        char Delim;
    };

    template <class TpFunc>
    struct TOptKVHandler: public IOptHandler {
    public:
        using TKey = typename TFunctionArgs<TpFunc>::template TGet<0>;
        using TValue = typename TFunctionArgs<TpFunc>::template TGet<1>;

        explicit TOptKVHandler(TpFunc func, const char kvdelim = '=')
            : Func(func)
            , KVDelim(kvdelim)
        {
        }

        void HandleOpt(const TOptsParser* parser) override {
            const std::string_view curval(parser->CurValOrDef());
            const TOpt* curOpt(parser->CurOpt());
            if (curval.data() != nullptr) {
                std::string_view key, value;
                if (!NUtils::TrySplit(curval, key, value, KVDelim)) {
                    throw TUsageException() << "failed to parse opt " << NPrivate::OptToString(curOpt)
                                             << " value " << NUtils::Quote(curval) << ": expected key" << KVDelim << "value format";
                }
                Func(NPrivate::OptFromString<TKey>(key, curOpt), NPrivate::OptFromString<TValue>(value, curOpt));
            }
        }

    private:
        TpFunc Func;
        char KVDelim;
    };

    namespace NPrivate {
        template <typename TpFunc, typename TpArg>
        void THandlerFunctor1<TpFunc, TpArg>::HandleOpt(const TOptsParser* parser) {
            const std::string_view curval = parser->CurValOrDef(!HasDef_);
            const TpArg& arg = curval.data() != nullptr ? OptFromString<TpArg>(curval, parser->CurOpt()) : Def_;
            try {
                Func_(arg);
            } catch (const TUsageException&) {
                throw;
            } catch (...) {
                throw TUsageException() << "failed to handle opt " << OptToString(parser->CurOpt())
                                         << " value " << NUtils::Quote(curval) << ": " << CurrentExceptionMessage();
            }
        }

    }

}
