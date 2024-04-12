#pragma once

#include <src/util/system/defaults.h>

#include <span>
#include <string_view>

class IInputStream;

class MD5 {
public:
    MD5() {
        Init();
    }

    void Init();

    inline MD5& Update(const void* data, size_t len) {
        return Update(std::span(static_cast<const ui8*>(data), len));
    }

    inline MD5& Update(std::string_view data) {
        return Update(data.data(), data.size());
    }

    inline MD5& Update(const std::span<const ui8> data) {
        UpdatePart(data);
        return *this;
    }

    void Pad();
    ui8* Final(ui8[16]);

    // buf must be char[33];
    char* End(char* buf);

    // buf must be char[25];
    char* End_b64(char* buf);

    // 8-byte xor-based mix
    ui64 EndHalfMix();

    MD5& Update(IInputStream* in);

    /*
     * Return hex-encoded md5 checksum for given file.
     *
     * Return nullptr / empty string if the file does not exist.
     */
    static char* File(const char* filename, char* buf);
    static std::string File(const std::string& filename);

    static char* Data(const void* data, size_t len, char* buf);
    static char* Data(const std::span<const ui8>& data, char* buf);
    static std::string Data(const std::span<const ui8>& data);
    static std::string Data(std::string_view data);
    static char* Stream(IInputStream* in, char* buf);

    static std::string Calc(std::string_view data);                     // 32-byte hex-encoded
    static std::string Calc(const std::span<const ui8>& data);    // 32-byte hex-encoded
    static std::string CalcRaw(std::string_view data);                  // 16-byte raw
    static std::string CalcRaw(const std::span<const ui8>& data); // 16-byte raw

    static ui64 CalcHalfMix(std::string_view data);
    static ui64 CalcHalfMix(const std::span<const ui8>& data);
    static ui64 CalcHalfMix(const char* data, size_t len);

    static bool IsMD5(std::string_view data);
    static bool IsMD5(const std::span<const ui8>& data);

private:
    void UpdatePart(std::span<const ui8> data);

private:
    ui8 BufferSize;  /* size of buffer */
    ui8 Buffer[64];  /* input buffer */
    ui32 State[4];   /* state (ABCD) */
    ui64 StreamSize; /* total bytes in input stream */
};
