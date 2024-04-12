#pragma once

#include "json_value.h"

<<<<<<<< HEAD:include/ydb-cpp-sdk/library/json/json_reader.h
#include <ydb-cpp-sdk/library/json/common/defs.h>
#include <ydb-cpp-sdk/library/json/fast_sax/parser.h>

#include <ydb-cpp-sdk/util/stream/mem.h>
========
#include <src/library/json/common/defs.h>
#include <src/library/json/fast_sax/parser.h>

#include <src/util/stream/mem.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/json/json_reader.h

namespace NJson {
    struct TJsonReaderConfig {
        TJsonReaderConfig();

        bool UseIterativeParser = false;
        // js-style comments (both // and /**/)
        bool AllowComments = false;
        bool DontValidateUtf8 = false;
        //bool AllowEscapedApostrophe = false;

        ui64 MaxDepth = 0;

        void SetBufferSize(size_t bufferSize);
        size_t GetBufferSize() const;

    private:
        size_t BufferSize;
    };

    bool ReadJsonTree(std::string_view in, TJsonValue* out, bool throwOnError = false);
    bool ReadJsonTree(std::string_view in, bool allowComments, TJsonValue* out, bool throwOnError = false);
    bool ReadJsonTree(std::string_view in, const TJsonReaderConfig* config, TJsonValue* out, bool throwOnError = false);

    bool ReadJsonTree(IInputStream* in, TJsonValue* out, bool throwOnError = false);
    bool ReadJsonTree(IInputStream* in, bool allowComments, TJsonValue* out, bool throwOnError = false);
    bool ReadJsonTree(IInputStream* in, const TJsonReaderConfig* config, TJsonValue* out, bool throwOnError = false);

    TJsonValue ReadJsonTree(IInputStream* in, bool throwOnError = false);
    TJsonValue ReadJsonTree(IInputStream* in, bool allowComments, bool throwOnError);
    TJsonValue ReadJsonTree(IInputStream* in, const TJsonReaderConfig* config, bool throwOnError = false);

    bool ReadJson(IInputStream* in, TJsonCallbacks* callbacks);
    bool ReadJson(IInputStream* in, bool allowComments, TJsonCallbacks* callbacks);
    //bool ReadJson(IInputStream* in, bool allowComments, bool allowEscapedApostrophe, TJsonCallbacks* callbacks);
    bool ReadJson(IInputStream* in, const TJsonReaderConfig* config, TJsonCallbacks* callbacks);

    enum ReaderConfigFlags {
        ITERATIVE = 0b1000,
        COMMENTS = 0b0100,
        VALIDATE = 0b0010,
        //ESCAPE = 0b0001,
    };

    inline bool ValidateJson(IInputStream* in, const TJsonReaderConfig* config, bool throwOnError = false) {
        TJsonCallbacks c(throwOnError);
        return ReadJson(in, config, &c);
    }

    inline bool ValidateJson(std::string_view in, const TJsonReaderConfig& config = TJsonReaderConfig(), bool throwOnError = false) {
        TMemoryInput min(in.data(), in.size());
        return ValidateJson(&min, &config, throwOnError);
    }

    inline bool ValidateJsonThrow(IInputStream* in, const TJsonReaderConfig* config) {
        return ValidateJson(in, config, true);
    }

    inline bool ValidateJsonThrow(std::string_view in, const TJsonReaderConfig& config = TJsonReaderConfig()) {
        return ValidateJson(in, config, true);
    }

    class TParserCallbacks: public TJsonCallbacks {
    public:
        TParserCallbacks(TJsonValue& value, bool throwOnError = false, bool notClosedBracketIsError = false);
        bool OnNull() override;
        bool OnBoolean(bool val) override;
        bool OnInteger(long long val) override;
        bool OnUInteger(unsigned long long val) override;
        bool OnString(const std::string_view& val) override;
        bool OnDouble(double val) override;
        bool OnOpenArray() override;
        bool OnCloseArray() override;
        bool OnOpenMap() override;
        bool OnCloseMap() override;
        bool OnMapKey(const std::string_view& val) override;
        bool OnEnd() override;

    protected:
        TJsonValue& Value;
        std::string Key;
        std::vector<TJsonValue*> ValuesStack;
        bool NotClosedBracketIsError;

        enum {
            START,
            AFTER_MAP_KEY,
            IN_MAP,
            IN_ARRAY,
            FINISH
        } CurrentState;

        template <class T>
        bool SetValue(const T& value) {
            switch (CurrentState) {
                case START:
                    Value.SetValue(value);
                    break;
                case AFTER_MAP_KEY:
                    ValuesStack.back()->InsertValue(Key, value);
                    CurrentState = IN_MAP;
                    break;
                case IN_ARRAY:
                    ValuesStack.back()->AppendValue(value);
                    break;
                case IN_MAP:
                case FINISH:
                    return false;
                default:
                    ythrow yexception() << "TParserCallbacks::SetValue invalid enum";
            }
            return true;
        }

        bool OpenComplexValue(EJsonValueType type);
        bool CloseComplexValue();
    };

    //// relaxed json, used in src/library/scheme
    bool ReadJsonFastTree(std::string_view in, TJsonValue* out, bool throwOnError = false, bool notClosedBracketIsError = false);
    TJsonValue ReadJsonFastTree(std::string_view in, bool notClosedBracketIsError = false);
}
