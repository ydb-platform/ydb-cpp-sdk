#pragma once

#include "public.h"

#include <src/util/generic/ptr.h>

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
