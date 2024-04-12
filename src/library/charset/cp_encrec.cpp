#include "codepage.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/output.h>
=======
#include <src/util/stream/output.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

void Encoder::Tr(const wchar32* in, char* out, size_t len) const {
    while (len--)
        *out++ = Tr(*in++);
}

void Encoder::Tr(const wchar32* in, char* out) const {
    do {
        *out++ = Tr(*in);
    } while (*in++);
}

void Recoder::Create(const CodePage& source, const Encoder* wideTarget) {
    for (size_t i = 0; i != 256; ++i) {
        Table[i] = wideTarget->Tr(source.unicode[i]);
        Y_ASSERT(Table[i] != 0 || i == 0);
    }
}

void Recoder::Create(const CodePage& page, const Encoder* widePage, wchar32 (*mapfunc)(wchar32)) {
    for (size_t i = 0; i != 256; ++i) {
        char c = widePage->Code((*mapfunc)(page.unicode[i]));
        Table[i] = (c == 0 && i != 0) ? (unsigned char)i : (unsigned char)c;
    }
}

void Recoder::Tr(const char* in, char* out, size_t len) const {
    while (len--)
        *out++ = Table[(unsigned char)*in++];
}

void Recoder::Tr(const char* in, char* out) const {
    do {
        *out++ = Table[(unsigned char)*in];
    } while (*in++);
}

void Recoder::Tr(char* in_out, size_t len) const {
    while (len--) {
        *in_out = Table[(unsigned char)*in_out];
        in_out++;
    }
}

void Recoder::Tr(char* in_out) const {
    // assuming that '\0' <--> '\0'
    do {
        *in_out = Table[(unsigned char)*in_out];
    } while (*in_out++);
}
