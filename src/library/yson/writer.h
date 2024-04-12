#pragma once

#include "public.h"
#include "token.h"
#include "consumer.h"

<<<<<<<< HEAD:include/ydb-cpp-sdk/library/yson/writer.h
#include <ydb-cpp-sdk/util/generic/noncopyable.h>
========
#include <src/util/generic/noncopyable.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/yson/writer.h

class IOutputStream;
class IZeroCopyInput;

namespace NYson {
    ////////////////////////////////////////////////////////////////////////////////

    class TYsonWriter
       : public TYsonConsumerBase,
          private TNonCopyable {
    public:
        class TState {
        private:
            int Depth;
            bool BeforeFirstItem;

            friend class TYsonWriter;
        };

    public:
        TYsonWriter(
            IOutputStream* stream,
            EYsonFormat format = EYsonFormat::Binary,
            EYsonType type = ::NYson::EYsonType::Node,
            bool enableRaw = false);

        void OnStringScalar(std::string_view value) override;
        void OnInt64Scalar(i64 value) override;
        void OnUint64Scalar(ui64 value) override;
        void OnDoubleScalar(double value) override;
        void OnBooleanScalar(bool value) override;
        void OnEntity() override;

        void OnBeginList() override;
        void OnListItem() override;
        void OnEndList() override;

        void OnBeginMap() override;
        void OnKeyedItem(std::string_view key) override;
        void OnEndMap() override;

        void OnBeginAttributes() override;
        void OnEndAttributes() override;

        void OnRaw(std::string_view yson, EYsonType type = ::NYson::EYsonType::Node) override;

        TState State() const;
        void Reset(const TState& state);

    protected:
        IOutputStream* Stream;
        EYsonFormat Format;
        EYsonType Type;
        bool EnableRaw;

        int Depth;
        bool BeforeFirstItem;

        static const int IndentSize = 4;

        void WriteIndent();
        void WriteStringScalar(const std::string_view& value);

        void BeginCollection(ETokenType beginToken);
        void CollectionItem(ETokenType separatorToken);
        void EndCollection(ETokenType endToken);

        bool IsTopLevelFragmentContext() const;
        void EndNode();
    };

    ////////////////////////////////////////////////////////////////////////////////

    void ReformatYsonStream(
        IInputStream* input,
        IOutputStream* output,
        EYsonFormat format = EYsonFormat::Binary,
        EYsonType type = ::NYson::EYsonType::Node);

    ////////////////////////////////////////////////////////////////////////////////

} // namespace NYson
