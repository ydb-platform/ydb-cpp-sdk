#include "join.h"
#include <ydb-cpp-sdk/util/string/subst.h>
#include <string>

#include <src/library/testing/unittest/registar.h>

Y_UNIT_TEST_SUITE(TStringSubst) {
    static const size_t MIN_FROM_CTX = 4;
    static const std::vector<std::string> ALL_FROM{std::string("F"), std::string("FF")};
    static const std::vector<std::string> ALL_TO{std::string(""), std::string("T"), std::string("TT"), std::string("TTT")};

    static void AssertSubstGlobal(const std::string& sFrom, const std::string& sTo, const std::string& from, const std::string& to, const size_t fromPos, const size_t numSubst) {
        std::string s = sFrom;
        size_t res = SubstGlobal(s, from, to, fromPos);
        UNIT_ASSERT_VALUES_EQUAL_C(res, numSubst,
                                   TStringBuilder() << "numSubst=" << numSubst << ", fromPos=" << fromPos << ", " << sFrom << " -> " << sTo);
        if (numSubst) {
            UNIT_ASSERT_STRINGS_EQUAL_C(s, sTo,
                                        TStringBuilder() << "numSubst=" << numSubst << ", fromPos=" << fromPos << ", " << sFrom << " -> " << sTo);
        } else {
            // ensure s didn't trigger copy-on-write
            UNIT_ASSERT_VALUES_EQUAL_C(s.c_str(), sFrom.c_str(),
                                       TStringBuilder() << "numSubst=" << numSubst << ", fromPos=" << fromPos << ", " << sFrom << " -> " << sTo);
        }
    }

    Y_UNIT_TEST(TestSubstGlobalNoSubstA) {
        for (const auto& from : ALL_FROM) {
            const size_t fromSz = from.size();
            const size_t minSz = fromSz;
            const size_t maxSz = fromSz + MIN_FROM_CTX;
            for (size_t sz = minSz; sz <= maxSz; ++sz) {
                for (size_t fromPos = 0; fromPos < sz; ++fromPos) {
                    std::string s{sz, '.'};
                    for (const auto& to : ALL_TO) {
                        AssertSubstGlobal(s, s, from, to, fromPos, 0);
                    }
                }
            }
        }
    }

    Y_UNIT_TEST(TestSubstGlobalNoSubstB) {
        for (const auto& from : ALL_FROM) {
            const size_t fromSz = from.size();
            const size_t minSz = fromSz;
            const size_t maxSz = fromSz + MIN_FROM_CTX;
            for (size_t sz = minSz; sz <= maxSz; ++sz) {
                for (size_t fromPos = 0; fromPos <= sz - fromSz; ++fromPos) {
                    for (size_t fromBeg = 0; fromBeg < fromPos; ++fromBeg) {
                        const auto parts = {
                            std::string(fromBeg, '.'),
                            std::string(sz - fromSz - fromBeg, '.')};
                        std::string s = JoinSeq(from, parts);
                        for (const auto& to : ALL_TO) {
                            AssertSubstGlobal(s, s, from, to, fromPos, 0);
                        }
                    }
                }
            }
        }
    }

    static void DoTestSubstGlobal(std::vector<std::string>& parts, const size_t minBeg, const size_t sz,
                                  const std::string& from, const size_t fromPos, const size_t numSubst) {
        const size_t numLeft = numSubst - parts.size();
        for (size_t fromBeg = minBeg; fromBeg <= sz - numLeft * from.size(); ++fromBeg) {
            if (parts.empty()) {
                parts.emplace_back(fromBeg, '.');
            } else {
                parts.emplace_back(fromBeg - minBeg, '.');
            }

            if (numLeft == 1) {
                parts.emplace_back(sz - fromBeg - from.size(), '.');
                std::string sFrom = JoinSeq(from, parts);
                UNIT_ASSERT_VALUES_EQUAL_C(sFrom.size(), sz, sFrom);
                for (const auto& to : ALL_TO) {
                    std::string sTo = JoinSeq(to, parts);
                    AssertSubstGlobal(sFrom, sTo, from, to, fromPos, numSubst);
                }
                parts.pop_back();
            } else {
                DoTestSubstGlobal(parts, fromBeg + from.size(), sz, from, fromPos, numSubst);
            }

            parts.pop_back();
        }
    }

    static void DoTestSubstGlobal(size_t numSubst) {
        std::vector<std::string> parts;
        for (const auto& from : ALL_FROM) {
            const size_t fromSz = from.size();
            const size_t minSz = numSubst * fromSz;
            const size_t maxSz = numSubst * (fromSz + MIN_FROM_CTX);
            for (size_t sz = minSz; sz <= maxSz; ++sz) {
                const size_t maxPos = sz - numSubst * fromSz;
                for (size_t fromPos = 0; fromPos <= maxPos; ++fromPos) {
                    DoTestSubstGlobal(parts, fromPos, sz, from, fromPos, numSubst);
                }
            }
        }
    }

    Y_UNIT_TEST(TestSubstGlobalSubst1) {
        DoTestSubstGlobal(1);
    }

    Y_UNIT_TEST(TestSubstGlobalSubst2) {
        DoTestSubstGlobal(2);
    }

    Y_UNIT_TEST(TestSubstGlobalSubst3) {
        DoTestSubstGlobal(3);
    }

    Y_UNIT_TEST(TestSubstGlobalSubst4) {
        DoTestSubstGlobal(4);
    }

    Y_UNIT_TEST(TestSubstGlobalOld) {
        std::string s;
        s = "aaa";
        SubstGlobal(s, "a", "bb");
        UNIT_ASSERT_EQUAL(s, std::string("bbbbbb"));
        s = "aaa";
        SubstGlobal(s, "a", "b");
        UNIT_ASSERT_EQUAL(s, std::string("bbb"));
        s = "aaa";
        SubstGlobal(s, "a", "");
        UNIT_ASSERT_EQUAL(s, std::string(""));
        s = "abcdefbcbcdfb";
        SubstGlobal(s, "bc", "bbc", 2);
        UNIT_ASSERT_EQUAL(s, std::string("abcdefbbcbbcdfb"));
        s = "Москва ~ Париж";
        SubstGlobal(s, " ~ ", " ");
        UNIT_ASSERT_EQUAL(s, std::string("Москва Париж"));
    }

    Y_UNIT_TEST(TestSubstGlobalOldRet) {
        const std::string s1 = "aaa";
        const std::string s2 = SubstGlobalCopy(s1, "a", "bb");
        UNIT_ASSERT_EQUAL(s2, std::string("bbbbbb"));

        const std::string s3 = "aaa";
        const std::string s4 = SubstGlobalCopy(s3, "a", "b");
        UNIT_ASSERT_EQUAL(s4, std::string("bbb"));

        const std::string s5 = "aaa";
        const std::string s6 = SubstGlobalCopy(s5, "a", "");
        UNIT_ASSERT_EQUAL(s6, std::string(""));

        const std::string s7 = "abcdefbcbcdfb";
        const std::string s8 = SubstGlobalCopy(s7, "bc", "bbc", 2);
        UNIT_ASSERT_EQUAL(s8, std::string("abcdefbbcbbcdfb"));

        const std::string s9 = "Москва ~ Париж";
        const std::string s10 = SubstGlobalCopy(s9, " ~ ", " ");
        UNIT_ASSERT_EQUAL(s10, std::string("Москва Париж"));
    }

    Y_UNIT_TEST(TestSubstCharGlobal) {
        std::u16string w = u"abcdabcd";
        SubstGlobal(w, wchar16('b'), wchar16('B'), 3);
        UNIT_ASSERT_EQUAL(w, u"abcdaBcd");

        std::string s = "aaa";
        SubstGlobal(s, 'a', 'b', 1);
        UNIT_ASSERT_EQUAL(s, std::string("abb"));
    }

    Y_UNIT_TEST(TestSubstCharGlobalRet) {
        const std::u16string w1 = u"abcdabcd";
        const std::u16string w2 = SubstGlobalCopy(w1, wchar16('b'), wchar16('B'), 3);
        UNIT_ASSERT_EQUAL(w2, u"abcdaBcd");

        const std::string s1 = "aaa";
        const std::string s2 = SubstGlobalCopy(s1, 'a', 'b', 1);
        UNIT_ASSERT_EQUAL(s2, std::string("abb"));
    }

    Y_UNIT_TEST(TestSubstStdString) {
        std::string s = "aaa";
        SubstGlobal(s, "a", "b", 1);
        UNIT_ASSERT_EQUAL(s, "abb");
    }

    Y_UNIT_TEST(TestSubstStdStringRet) {
        const std::string s1 = "aaa";
        const std::string s2 = SubstGlobalCopy(s1, "a", "b", 1);
        UNIT_ASSERT_EQUAL(s2, "abb");
    }

    Y_UNIT_TEST(TestSubstGlobalChar) {
        {
            const std::string s = "a";
            const std::string st = "b";
            std::string ss = s;
            UNIT_ASSERT_VALUES_EQUAL(s.size(), SubstGlobal(ss, 'a', 'b'));
            UNIT_ASSERT_VALUES_EQUAL(st, ss);
        }
        {
            const std::string s = "aa";
            const std::string st = "bb";
            std::string ss = s;
            UNIT_ASSERT_VALUES_EQUAL(s.size(), SubstGlobal(ss, 'a', 'b'));
            UNIT_ASSERT_VALUES_EQUAL(st, ss);
        }
        {
            const std::string s = "aaa";
            const std::string st = "bbb";
            std::string ss = s;
            UNIT_ASSERT_VALUES_EQUAL(s.size(), SubstGlobal(ss, 'a', 'b'));
            UNIT_ASSERT_VALUES_EQUAL(st, ss);
        }
        {
            const std::string s = "aaaa";
            const std::string st = "bbbb";
            std::string ss = s;
            UNIT_ASSERT_VALUES_EQUAL(s.size(), SubstGlobal(ss, 'a', 'b'));
            UNIT_ASSERT_VALUES_EQUAL(st, ss);
        }
        {
            const std::string s = "aaaaa";
            const std::string st = "bbbbb";
            std::string ss = s;
            UNIT_ASSERT_VALUES_EQUAL(s.size(), SubstGlobal(ss, 'a', 'b'));
            UNIT_ASSERT_VALUES_EQUAL(st, ss);
        }
        {
            const std::string s = "aaaaaa";
            const std::string st = "bbbbbb";
            std::string ss = s;
            UNIT_ASSERT_VALUES_EQUAL(s.size(), SubstGlobal(ss, 'a', 'b'));
            UNIT_ASSERT_VALUES_EQUAL(st, ss);
        }
        {
            const std::string s = "aaaaaaa";
            const std::string st = "bbbbbbb";
            std::string ss = s;
            UNIT_ASSERT_VALUES_EQUAL(s.size(), SubstGlobal(ss, 'a', 'b'));
            UNIT_ASSERT_VALUES_EQUAL(st, ss);
        }
        {
            const std::string s = "aaaaaaaa";
            const std::string st = "bbbbbbbb";
            std::string ss = s;
            UNIT_ASSERT_VALUES_EQUAL(s.size(), SubstGlobal(ss, 'a', 'b'));
            UNIT_ASSERT_VALUES_EQUAL(st, ss);
        }
    }
}
