#pragma once

#include <ydb-cpp-sdk/library/yson/public.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

class IInputStream;

namespace NYT::NYson {
struct IYsonConsumer;
} // namespace NYT::NYson

namespace NYson {
    ////////////////////////////////////////////////////////////////////////////////

    class TYsonParser {
    public:
        TYsonParser(
            NYT::NYson::IYsonConsumer* consumer,
            IInputStream* stream,
            EYsonType type = ::NYson::EYsonType::Node,
            bool enableLinePositionInfo = false,
            std::optional<ui64> memoryLimit = std::nullopt);

        ~TYsonParser();

        void Parse();

    private:
        class TImpl;
        THolder<TImpl> Impl;
    };

    ////////////////////////////////////////////////////////////////////////////////

    class TStatelessYsonParser {
    public:
        TStatelessYsonParser(
            NYT::NYson::IYsonConsumer* consumer,
            bool enableLinePositionInfo = false,
            std::optional<ui64> memoryLimit = std::nullopt);

        ~TStatelessYsonParser();

        void Parse(const std::string_view& data, EYsonType type = ::NYson::EYsonType::Node);

    private:
        class TImpl;
        THolder<TImpl> Impl;
    };

    ////////////////////////////////////////////////////////////////////////////////

    class TYsonListParser {
    public:
        TYsonListParser(
            NYT::NYson::IYsonConsumer* consumer,
            IInputStream* stream,
            bool enableLinePositionInfo = false,
            std::optional<ui64> memoryLimit = std::nullopt);

        ~TYsonListParser();

        bool Parse(); // Returns false, if there is no more list items

    private:
        class TImpl;
        THolder<TImpl> Impl;
    };

    ////////////////////////////////////////////////////////////////////////////////

    void ParseYsonStringBuffer(
        const std::string_view& buffer,
        NYT::NYson::IYsonConsumer* consumer,
        EYsonType type = ::NYson::EYsonType::Node,
        bool enableLinePositionInfo = false,
        std::optional<ui64> memoryLimit = std::nullopt);

    ////////////////////////////////////////////////////////////////////////////////

} // namespace NYson
