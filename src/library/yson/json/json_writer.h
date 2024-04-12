#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/library/yson/public.h>
#include <ydb-cpp-sdk/library/yson/consumer.h>
=======
#include <src/library/yson/public.h>
#include <src/library/yson/consumer.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

#include <src/library/json/json_writer.h>



namespace NYT {
    ////////////////////////////////////////////////////////////////////////////////

    enum EJsonFormat {
        JF_TEXT,
        JF_PRETTY
    };

    enum EJsonAttributesMode {
        JAM_NEVER,
        JAM_ON_DEMAND,
        JAM_ALWAYS
    };

    enum ESerializedBoolFormat {
        SBF_BOOLEAN,
        SBF_STRING
    };

    class TJsonWriter
       : public ::NYson::TYsonConsumerBase {
    public:
        TJsonWriter(
            IOutputStream* output,
            ::NYson::EYsonType type = ::NYson::EYsonType::Node,
            EJsonFormat format = JF_TEXT,
            EJsonAttributesMode attributesMode = JAM_ON_DEMAND,
            ESerializedBoolFormat booleanFormat = SBF_STRING);

        TJsonWriter(
            IOutputStream* output,
            NJson::TJsonWriterConfig config,
            ::NYson::EYsonType type = ::NYson::EYsonType::Node,
            EJsonAttributesMode attributesMode = JAM_ON_DEMAND,
            ESerializedBoolFormat booleanFormat = SBF_STRING);

        void Flush();

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

    private:
        THolder<NJson::TJsonWriter> UnderlyingJsonWriter;
        NJson::TJsonWriter* JsonWriter;
        IOutputStream* Output;
        ::NYson::EYsonType Type;
        EJsonAttributesMode AttributesMode;
        ESerializedBoolFormat BooleanFormat;

        void WriteStringScalar(const std::string_view& value);

        void EnterNode();
        void LeaveNode();
        bool IsWriteAllowed();

        std::vector<bool> HasUnfoldedStructureStack;
        int InAttributesBalance;
        bool HasAttributes;
        int Depth;
    };

    ////////////////////////////////////////////////////////////////////////////////

}
