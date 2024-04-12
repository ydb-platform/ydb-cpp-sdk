#pragma once

// Deprecated. Use src/library/json/writer in new code.

#include "json_value.h"

#include <src/library/json/writer/json.h>

#include <util/stream/output.h>
#include <util/generic/hash.h>

#include <string>

namespace NJson {
    struct TJsonWriterConfig {
        constexpr static ui32 DefaultDoubleNDigits = 10;
        constexpr static ui32 DefaultFloatNDigits = 6;
        constexpr static EFloatToStringMode DefaultFloatToStringMode = PREC_NDIGITS;

        inline TJsonWriterConfig& SetUnbuffered(bool v) noexcept {
            Unbuffered = v;

            return *this;
        }

        inline TJsonWriterConfig& SetValidateUtf8(bool v) noexcept {
            ValidateUtf8 = v;

            return *this;
        }

        inline TJsonWriterConfig& SetFormatOutput(bool v) noexcept {
            FormatOutput = v;

            return *this;
        }

        ui32 DoubleNDigits = DefaultDoubleNDigits;
        ui32 FloatNDigits = DefaultFloatNDigits;
        EFloatToStringMode FloatToStringMode = DefaultFloatToStringMode;
        bool FormatOutput = false;
        bool SortKeys = false;
        bool ValidateUtf8 = true;
        bool DontEscapeStrings = false;
        bool Unbuffered = false;
        bool WriteNanAsString = false; // NaN and Inf are not valid json values, so if WriteNanAsString is set, writer would write string intead of throwing exception (default case)
    };

    class TJsonWriter {
        IOutputStream* Out;
        NJsonWriter::TBuf Buf;
        const ui32 DoubleNDigits;
        const ui32 FloatNDigits;
        const EFloatToStringMode FloatToStringMode;
        const bool SortKeys;
        const bool ValidateUtf8;
        const bool DontEscapeStrings;
        const bool DontFlushInDestructor;

    public:
        TJsonWriter(IOutputStream* out, bool formatOutput, bool sortkeys = false, bool validateUtf8 = true);
        TJsonWriter(IOutputStream* out, const TJsonWriterConfig& config, bool DontFlushInDestructor = false);
        ~TJsonWriter();

        void Flush();

        void OpenMap();
        void OpenMap(const std::string_view& key) {
            Buf.WriteKey(key);
            OpenMap();
        }
        void CloseMap();

        void OpenArray();
        void OpenArray(const std::string_view& key) {
            Buf.WriteKey(key);
            OpenArray();
        }
        void CloseArray();

        void WriteNull();

        void Write(const std::string_view& value);
        void Write(float value);
        void Write(double value);
        void Write(bool value);
        void Write(const TJsonValue* value);
        void Write(const TJsonValue& value);

        // must use all variations of integer types since long
        // and long long are different types but with same size
        void Write(long long value);
        void Write(unsigned long long value);
        void Write(long value) {
            Write((long long)value);
        }
        void Write(unsigned long value) {
            Write((unsigned long long)value);
        }
        void Write(int value) {
            Write((long long)value);
        }
        void Write(unsigned int value) {
            Write((unsigned long long)value);
        }
        void Write(short value) {
            Write((long long)value);
        }
        void Write(unsigned short value) {
            Write((unsigned long long)value);
        }

        void Write(const unsigned char* value) {
            Write((const char*)value);
        }
        void Write(const char* value) {
            Write(std::string_view(value));
        }
        void Write(const std::string& value) {
            Write(std::string_view(value));
        }

        // write raw json without checks
        void UnsafeWrite(const std::string_view& value) {
            Buf.UnsafeWriteValue(value);
        }

        template <typename T>
        void Write(const std::string_view& key, const T& value) {
            Buf.WriteKey(key);
            Write(value);
        }

        // write raw json without checks
        void UnsafeWrite(const std::string_view& key, const std::string_view& value) {
            Buf.WriteKey(key);
            UnsafeWrite(value);
        }

        void WriteNull(const std::string_view& key) {
            Buf.WriteKey(key);
            WriteNull();
        }

        template <typename T>
        void WriteOptional(const std::string_view& key, const std::optional<T>& value) {
            if (value) {
                Write(key, *value);
            }
        }

        void WriteOptional(const std::string_view&, const std::nullopt_t&) {
            // nothing to do
        }

        void WriteKey(const std::string_view key) {
            Buf.WriteKey(key);
        }

        void WriteKey(const unsigned char* key) {
            WriteKey((const char*)key);
        }

        void WriteKey(const char* key) {
            WriteKey(std::string_view{key});
        }

        void WriteKey(const std::string& key) {
            WriteKey(std::string_view{key});
        }

        NJsonWriter::TBufState State() const {
            return Buf.State();
        }

        void Reset(const NJsonWriter::TBufState& from) {
            return Buf.Reset(from);
        }

        void Reset(NJsonWriter::TBufState&& from) {
            return Buf.Reset(std::move(from));
        }
    };

    void WriteJson(IOutputStream*, const TJsonValue*, bool formatOutput = false, bool sortkeys = false, bool validateUtf8 = true);
    std::string WriteJson(const TJsonValue*, bool formatOutput = true, bool sortkeys = false, bool validateUtf8 = false);
    std::string WriteJson(const TJsonValue&, bool formatOutput = true, bool sortkeys = false, bool validateUtf8 = false);
    void WriteJson(IOutputStream*, const TJsonValue*, const TJsonWriterConfig& config);
}
