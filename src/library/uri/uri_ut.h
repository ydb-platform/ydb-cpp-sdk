#pragma once

#include "uri.h"

#include <src/library/testing/unittest/registar.h>

namespace NUri {
    struct TTest {
        std::string_view Val;
        TParseFlags Flags;
        TState::EParsed State;
        std::string_view Scheme;
        std::string_view User;
        std::string_view Pass;
        std::string_view Host;
        ui16 Port;
        std::string_view Path;
        std::string_view Query;
        std::string_view Frag;
        std::string_view HashBang;
    };

}

#define URL_MSG(url1, url2, cmp) \
    (std::string("[") + url1.PrintS() + ("] " cmp " [") + url2.PrintS() + "]")
#define URL_EQ(url1, url2) \
    UNIT_ASSERT_EQUAL_C(url, url2, URL_MSG(url1, url2, "!="))
#define URL_NEQ(url1, url2) \
    UNIT_ASSERT_UNEQUAL_C(url, url2, URL_MSG(url1, url2, "=="))

#define CMP_FLD(url, test, fld) \
    UNIT_ASSERT_VALUES_EQUAL(url.GetField(TField::Field##fld), test.fld)

#define CMP_URL(url, test)                                  \
    do {                                                    \
        CMP_FLD(url, test, Scheme);                         \
        CMP_FLD(url, test, User);                           \
        CMP_FLD(url, test, Pass);                           \
        CMP_FLD(url, test, Host);                           \
        UNIT_ASSERT_VALUES_EQUAL(url.GetPort(), test.Port); \
        CMP_FLD(url, test, Path);                           \
        CMP_FLD(url, test, Query);                          \
        CMP_FLD(url, test, Frag);                           \
        CMP_FLD(url, test, HashBang);                       \
    } while (false)

#define URL_TEST_ENC(url, test, enc)                                                                                              \
    do {                                                                                                                          \
        TState::EParsed st = url.ParseUri(test.Val, test.Flags, 0, enc);                                                          \
        UNIT_ASSERT_VALUES_EQUAL(st, test.State);                                                                                 \
        CMP_URL(url, test);                                                                                                       \
        if (TState::ParsedOK != st)                                                                                               \
            break;                                                                                                                \
        TUri _url;                                                                                                                \
        std::string urlstr, urlstr2;                                                                                                  \
        urlstr = url.PrintS();                                                                                                    \
        TState::EParsed st2 = _url.ParseUri(urlstr,                                                                               \
                                            (test.Flags & ~TFeature::FeatureNoRelPath) | TFeature::FeatureAllowRootless, 0, enc); \
        if (TState::ParsedEmpty != st2)                                                                                           \
            UNIT_ASSERT_VALUES_EQUAL(st2, test.State);                                                                            \
        urlstr2 = _url.PrintS();                                                                                                  \
        UNIT_ASSERT_VALUES_EQUAL(urlstr, urlstr2);                                                                                \
        CMP_URL(_url, test);                                                                                                      \
        UNIT_ASSERT_VALUES_EQUAL(url.GetUrlFieldMask(), _url.GetUrlFieldMask());                                                  \
        URL_EQ(url, _url);                                                                                                        \
        const std::string_view hostascii = url.GetField(TField::FieldHostAscii);                                                        \
        if (hostascii.Empty())                                                                                                    \
            break;                                                                                                                \
        urlstr = url.PrintS(TField::FlagHostAscii);                                                                               \
        st2 = _url.ParseUri(urlstr,                                                                                               \
                            (test.Flags & ~TFeature::FeatureNoRelPath) | TFeature::FeatureAllowRootless, 0, enc);                 \
        UNIT_ASSERT_VALUES_EQUAL(st2, test.State);                                                                                \
        urlstr2 = _url.PrintS();                                                                                                  \
        UNIT_ASSERT_VALUES_EQUAL(urlstr, urlstr2);                                                                                \
        TTest test2 = test;                                                                                                       \
        test2.Host = hostascii;                                                                                                   \
        CMP_URL(_url, test2);                                                                                                     \
        UNIT_ASSERT_VALUES_EQUAL(url.GetUrlFieldMask(), _url.GetUrlFieldMask());                                                  \
    } while (false)

#define URL_TEST(url, test) \
    URL_TEST_ENC(url, test, CODES_UTF8)
