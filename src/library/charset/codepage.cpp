#include "ci_string.h"
#include "codepage.h"

#include <ydb-cpp-sdk/util/string/cast.h>
#include <ydb-cpp-sdk/util/string/subst.h>
#include <ydb-cpp-sdk/util/system/yassert.h>
#include <ydb-cpp-sdk/util/generic/singleton.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>

#include <src/util/string/util.h>
#include <src/util/system/hi_lo.h>
#include <src/util/memory/pool.h>

#include <cstring>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <ctype.h>

using namespace NCodepagePrivate;

void Recoder::Create(const CodePage& source, const CodePage& target) {
    const Encoder* wideTarget = &EncoderByCharset(target.CPEnum);
    Create(source, wideTarget);
}
void Recoder::Create(const CodePage& page, wchar32 (*mapfunc)(wchar32)) {
    const Encoder* widePage = &EncoderByCharset(page.CPEnum);
    Create(page, widePage, mapfunc);
}

template <class T, class T1>
static inline T1 Apply(T b, T e, T1 to, const Recoder& mapper) {
    while (b != e) {
        *to++ = mapper.Table[(unsigned char)*b++];
    }

    return to;
}

template <class T, class T1>
static inline T1 Apply(T b, T1 to, const Recoder& mapper) {
    while (*b != 0) {
        *to++ = mapper.Table[(unsigned char)*b++];
    }

    return to;
}

char* CodePage::ToLower(const char* b, const char* e, char* to) const {
    return Apply(b, e, to, TCodePageData::rcdr_to_lower[CPEnum]);
}
char* CodePage::ToLower(const char* b, char* to) const {
    return Apply(b, to, TCodePageData::rcdr_to_lower[CPEnum]);
}

char* CodePage::ToUpper(const char* b, const char* e, char* to) const {
    return Apply(b, e, to, TCodePageData::rcdr_to_upper[CPEnum]);
}
char* CodePage::ToUpper(const char* b, char* to) const {
    return Apply(b, to, TCodePageData::rcdr_to_upper[CPEnum]);
}

int CodePage::Stricmp(const char* dst, const char* src) const {
    unsigned char f, l;
    do {
        f = ToLower(*dst++);
        l = ToLower(*src++);
    } while (f && (f == l));
    return f - l;
}

int CodePage::Strnicmp(const char* dst, const char* src, size_t len) const {
    unsigned char f, l;
    if (len) {
        do {
            f = ToLower(*dst++);
            l = ToLower(*src++);
        } while (--len && f && (f == l));
        return f - l;
    }
    return 0;
}

static const CodePage UNSUPPORTED_CODEPAGE = {
    CODES_UNSUPPORTED,
    {
        "unsupported",
    },
    {},
    nullptr,
};

static const CodePage UNKNOWN_CODEPAGE = {
    CODES_UNKNOWN,
    {
        "unknown",
    },
    {},
    nullptr,
};

void NCodepagePrivate::TCodepagesMap::SetData(const CodePage* cp) {
    Y_ASSERT(cp);
    int code = static_cast<int>(cp->CPEnum) + DataShift;

    Y_ASSERT(code >= 0 && code < DataSize);
    Y_ASSERT(Data[code] == nullptr);

    Data[code] = cp;
}

NCodepagePrivate::TCodepagesMap::TCodepagesMap() {
    memset(Data, 0, sizeof(const CodePage*) * DataSize);
    SetData(&UNSUPPORTED_CODEPAGE);
    SetData(&UNKNOWN_CODEPAGE);

    for (size_t i = 0; i != CODES_MAX; ++i) {
        SetData(TCodePageData::AllCodePages[i]);
    }
}

const NCodepagePrivate::TCodepagesMap& NCodepagePrivate::TCodepagesMap::Instance() {
    return *Singleton<NCodepagePrivate::TCodepagesMap>();
}

class TCodePageHash {
private:
    using TData = std::unordered_map<std::string_view, ECharset, ci_hash, ci_equal_to>;

    TData Data;
    TMemoryPool Pool;

private:
    inline void AddNameWithCheck(const std::string& name, ECharset code) {
        if (Data.find(name.c_str()) == Data.end()) {
            Data.insert(TData::value_type(Pool.Append(name.data(), name.size() + 1), code));
        } else {
            Y_ASSERT(Data.find(name.c_str())->second == code);
        }
    }

    inline void AddName(const std::string& name, ECharset code) {
        AddNameWithCheck(name, code);

        std::string temp = name;
        NUtils::RemoveAll(temp, '-');
        NUtils::RemoveAll(temp, '_');
        AddNameWithCheck(temp, code);

        temp = name;
        SubstGlobal(temp, '-', '_');
        AddNameWithCheck(temp, code);

        temp = name;
        SubstGlobal(temp, '_', '-');
        AddNameWithCheck(temp, code);
    }

public:
    inline TCodePageHash()
        : Pool(20 * 1024) /* Currently used: 17KB. */
    {
        std::string xPrefix = "x-";
        const char* name;

        for (size_t i = 0; i != CODES_MAX; ++i) {
            ECharset e = static_cast<ECharset>(i);
            const CodePage* page = Singleton<NCodepagePrivate::TCodepagesMap>()->GetPrivate(e);

            AddName(ToString(static_cast<int>(i)), e);

            for (size_t j = 0; (name = page->Names[j]) != nullptr && name[0]; ++j) {
                AddName(name, e);

                AddName(xPrefix + name, e);
            }
        }
    }

    inline ECharset CharsetByName(std::string_view name) {
        if (name.empty()) {
            return CODES_UNKNOWN;
        }

        auto it = Data.find(name);
        if (it == Data.end()) {
            return CODES_UNKNOWN;
        }

        return it->second;
    }
};

ECharset CharsetByName(std::string_view name) {
    return Singleton<TCodePageHash>()->CharsetByName(name);
}

ECharset CharsetByNameOrDie(std::string_view name) {
    ECharset result = CharsetByName(name);
    if (result == CODES_UNKNOWN) {
        ythrow yexception() << "CharsetByNameOrDie: unknown charset '" << name << "'";
    }
    return result;
}

class TWindowsPrefixesHashSet: public std::unordered_set<std::string> {
public:
    inline TWindowsPrefixesHashSet() {
        insert("win");
        insert("wincp");
        insert("window");
        insert("windowcp");
        insert("windows");
        insert("windowscp");
        insert("ansi");
        insert("ansicp");
    }
};

class TCpPrefixesHashSet: public std::unordered_set<std::string> {
public:
    inline TCpPrefixesHashSet() {
        insert("microsoft");
        insert("microsoftcp");
        insert("cp");
    }
};

class TIsoPrefixesHashSet: public std::unordered_set<std::string> {
public:
    inline TIsoPrefixesHashSet() {
        insert("iso");
        insert("isolatin");
        insert("latin");
    }
};

class TLatinToIsoHash: public std::unordered_map<const char*, std::string, ci_hash, ci_equal_to> {
public:
    inline TLatinToIsoHash() {
        insert(value_type("latin1", "iso-8859-1"));
        insert(value_type("latin2", "iso-8859-2"));
        insert(value_type("latin3", "iso-8859-3"));
        insert(value_type("latin4", "iso-8859-4"));
        insert(value_type("latin5", "iso-8859-9"));
        insert(value_type("latin6", "iso-8859-10"));
        insert(value_type("latin7", "iso-8859-13"));
        insert(value_type("latin8", "iso-8859-14"));
        insert(value_type("latin9", "iso-8859-15"));
        insert(value_type("latin10", "iso-8859-16"));
    }
};

static inline void NormalizeEncodingPrefixes(std::string& enc) {
    const auto preflen = enc.find_first_of("0123456789");
    if (preflen == std::string::npos) {
        return;
    }

    std::string prefix = enc.substr(0, preflen);
    for (size_t i = 0; i < prefix.length(); ++i) {
        if (prefix[i] == '-') {
            prefix.erase(i--);
        }
    }

    if (Singleton<TWindowsPrefixesHashSet>()->contains(prefix)) {
        enc.erase(0, preflen);
        enc.insert(0, "windows-");
        return;
    }

    if (Singleton<TCpPrefixesHashSet>()->contains(prefix)) {
        if (enc.length() > preflen + 3 && !strncmp(enc.c_str() + preflen, "125", 3) && isdigit(enc[preflen + 3])) {
            enc.erase(0, preflen);
            enc.insert(0, "windows-");
            return;
        }
        enc.erase(0, preflen);
        enc.insert(0, "cp");
        return;
    }

    if (Singleton<TIsoPrefixesHashSet>()->contains(prefix)) {
        if (enc.length() == preflen + 1 || enc.length() == preflen + 2) {
            std::string enccopy = enc.substr(preflen);
            enccopy.insert(0, "latin");
            const TLatinToIsoHash* latinhash = Singleton<TLatinToIsoHash>();
            if (auto it = latinhash->find(enccopy.data()); it != latinhash->end()) {
                enc.assign(it->second);
            }
            return;
        } else if (enc.length() > preflen + 5 && enc[preflen] == '8') {
            enc.erase(0, preflen);
            enc.insert(0, "iso-");
            return;
        }
    }
}

class TEncodingNamesHashSet: public std::unordered_set<std::string> {
public:
    TEncodingNamesHashSet() {
        insert("iso-8859-1");
        insert("iso-8859-2");
        insert("iso-8859-3");
        insert("iso-8859-4");
        insert("iso-8859-5");
        insert("iso-8859-6");
        insert("iso-8859-7");
        insert("iso-8859-8");
        insert("iso-8859-8-i");
        insert("iso-8859-9");
        insert("iso-8859-10");
        insert("iso-8859-11");
        insert("iso-8859-12");
        insert("iso-8859-13");
        insert("iso-8859-14");
        insert("iso-8859-15");
        insert("windows-1250");
        insert("windows-1251");
        insert("windows-1252");
        insert("windows-1253");
        insert("windows-1254");
        insert("windows-1255");
        insert("windows-1256");
        insert("windows-1257");
        insert("windows-1258");
        insert("windows-874");
        insert("iso-2022-jp");
        insert("euc-jp");
        insert("shift-jis");
        insert("shiftjis");
        insert("iso-2022-kr");
        insert("euc-kr");
        insert("gb-2312");
        insert("gb2312");
        insert("gb-18030");
        insert("gb18030");
        insert("gbk");
        insert("big5");
        insert("tis-620");
        insert("tis620");
    }
};

ECharset EncodingHintByName(const char* encname) {
    if (!encname) {
        return CODES_UNKNOWN; // safety check
    }

    // Common trouble: spurious "charset=" in the encoding name
    if (!strnicmp(encname, "charset=", 8)) {
        encname += 8;
    }

    // Strip everything up to the first alphanumeric, and after the last one
    while (*encname && !isalnum(*encname)) {
        ++encname;
    }

    if (!*encname) {
        return CODES_UNKNOWN;
    }

    const char* lastpos = encname + strlen(encname) - 1;
    while (lastpos > encname && !isalnum(*lastpos)) {
        --lastpos;
    }

    // Do some normalization
    std::string enc(encname, lastpos - encname + 1);
    NUtils::ToLower(enc);
    for (char* p = enc.data(); p != enc.data() + enc.size(); ++p) {
        if (*p == ' ' || *p == '=' || *p == '_') {
            *p = '-';
        }
    }

    NormalizeEncodingPrefixes(enc);

    ECharset hint = CharsetByName(enc.c_str());
    if (hint != CODES_UNKNOWN) {
        return hint;
    }

    if (Singleton<TEncodingNamesHashSet>()->contains(enc)) {
        return CODES_UNSUPPORTED;
    }
    return CODES_UNKNOWN;
}
