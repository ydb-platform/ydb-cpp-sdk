#include <library/cpp/testing/unittest/registar.h>

Y_UNIT_TEST_SUITE(TUnitTestMacroTest) {
    Y_UNIT_TEST(Assert) {
        auto unitAssert = [] {
            UNIT_ASSERT(false);
        };
        UNIT_ASSERT_TEST_FAILS(unitAssert());

        UNIT_ASSERT(true);
    }

    Y_UNIT_TEST(TypesEqual) {
        auto typesEqual = [] {
            UNIT_ASSERT_TYPES_EQUAL(int, long);
        };
        UNIT_ASSERT_TEST_FAILS(typesEqual());

        UNIT_ASSERT_TYPES_EQUAL(std::string, std::string);
    }

    Y_UNIT_TEST(DoublesEqual) {
        auto doublesEqual = [](double d1, double d2, double precision) {
            UNIT_ASSERT_DOUBLES_EQUAL(d1, d2, precision);
        };
        UNIT_ASSERT_TEST_FAILS(doublesEqual(0.0, 0.5, 0.1));
        UNIT_ASSERT_TEST_FAILS(doublesEqual(0.1, -0.1, 0.1));

        UNIT_ASSERT_DOUBLES_EQUAL(0.0, 0.01, 0.1);
        UNIT_ASSERT_DOUBLES_EQUAL(0.01, 0.0, 0.1);

        constexpr auto nan = std::numeric_limits<double>::quiet_NaN();
        UNIT_ASSERT_TEST_FAILS(doublesEqual(nan, 0.5, 0.1));
        UNIT_ASSERT_TEST_FAILS(doublesEqual(0.5, nan, 0.1));
        UNIT_ASSERT_DOUBLES_EQUAL(nan, nan, 0.1);
    }

    Y_UNIT_TEST(StringsEqual) {
        auto stringsEqual = [](auto s1, auto s2) {
            UNIT_ASSERT_STRINGS_EQUAL(s1, s2);
        };
        UNIT_ASSERT_TEST_FAILS(stringsEqual("q", "w"));
        UNIT_ASSERT_TEST_FAILS(stringsEqual("q", std::string("w")));
        UNIT_ASSERT_TEST_FAILS(stringsEqual(std::string("q"), "w"));
        UNIT_ASSERT_TEST_FAILS(stringsEqual(std::string("a"), std::string("b")));
        UNIT_ASSERT_TEST_FAILS(stringsEqual(std::string("a"), std::string_view("b")));
        UNIT_ASSERT_TEST_FAILS(stringsEqual("a", std::string_view("b")));
        UNIT_ASSERT_TEST_FAILS(stringsEqual(std::string_view("a"), "b"));

        std::string empty;
        std::string_view emptyBuf;
        UNIT_ASSERT_STRINGS_EQUAL("", empty);
        UNIT_ASSERT_STRINGS_EQUAL(empty, emptyBuf);
        UNIT_ASSERT_STRINGS_EQUAL("", static_cast<const char*>(nullptr));
    }

    Y_UNIT_TEST(StringContains) {
        auto stringContains = [](auto s, auto substr) {
            UNIT_ASSERT_STRING_CONTAINS(s, substr);
        };
        UNIT_ASSERT_TEST_FAILS(stringContains("", "a"));
        UNIT_ASSERT_TEST_FAILS(stringContains("lurkmore", "moar"));

        UNIT_ASSERT_STRING_CONTAINS("", "");
        UNIT_ASSERT_STRING_CONTAINS("a", "");
        UNIT_ASSERT_STRING_CONTAINS("failure", "fail");
        UNIT_ASSERT_STRING_CONTAINS("lurkmore", "more");
    }

    Y_UNIT_TEST(NoDiff) {
        auto noDiff = [](auto s1, auto s2) {
            UNIT_ASSERT_NO_DIFF(s1, s2);
        };
        UNIT_ASSERT_TEST_FAILS(noDiff("q", "w"));
        UNIT_ASSERT_TEST_FAILS(noDiff("q", ""));

        UNIT_ASSERT_NO_DIFF("", "");
        UNIT_ASSERT_NO_DIFF("a", "a");
    }

    Y_UNIT_TEST(StringsUnequal) {
        auto stringsUnequal = [](auto s1, auto s2) {
            UNIT_ASSERT_STRINGS_UNEQUAL(s1, s2);
        };
        UNIT_ASSERT_TEST_FAILS(stringsUnequal("1", "1"));
        UNIT_ASSERT_TEST_FAILS(stringsUnequal("", ""));
        UNIT_ASSERT_TEST_FAILS(stringsUnequal("42", std::string("42")));
        UNIT_ASSERT_TEST_FAILS(stringsUnequal(std::string("4"), "4"));
        UNIT_ASSERT_TEST_FAILS(stringsUnequal("d", std::string_view("d")));
        UNIT_ASSERT_TEST_FAILS(stringsUnequal(std::string_view("yandex"), "yandex"));
        UNIT_ASSERT_TEST_FAILS(stringsUnequal(std::string_view("index"), std::string("index")));
        UNIT_ASSERT_TEST_FAILS(stringsUnequal(std::string("diff"), std::string_view("diff")));

        UNIT_ASSERT_STRINGS_UNEQUAL("1", "2");
        UNIT_ASSERT_STRINGS_UNEQUAL("", "3");
        UNIT_ASSERT_STRINGS_UNEQUAL("green", std::string_view("red"));
        UNIT_ASSERT_STRINGS_UNEQUAL(std::string_view("solomon"), "golovan");
        UNIT_ASSERT_STRINGS_UNEQUAL("d", std::string("f"));
        UNIT_ASSERT_STRINGS_UNEQUAL(std::string("yandex"), "index");
        UNIT_ASSERT_STRINGS_UNEQUAL(std::string("mail"), std::string_view("yandex"));
        UNIT_ASSERT_STRINGS_UNEQUAL(std::string_view("C++"), std::string("python"));
    }

    Y_UNIT_TEST(Equal) {
        auto equal = [](auto v1, auto v2) {
            UNIT_ASSERT_EQUAL(v1, v2);
        };
        UNIT_ASSERT_TEST_FAILS(equal("1", std::string("2")));
        UNIT_ASSERT_TEST_FAILS(equal(1, 2));
        UNIT_ASSERT_TEST_FAILS(equal(42ul, static_cast<unsigned short>(24)));

        UNIT_ASSERT_EQUAL("abc", std::string("abc"));
        UNIT_ASSERT_EQUAL(12l, 12);
        UNIT_ASSERT_EQUAL(55, 55);
    }

    Y_UNIT_TEST(Unequal) {
        auto unequal = [](auto v1, auto v2) {
            UNIT_ASSERT_UNEQUAL(v1, v2);
        };
        UNIT_ASSERT_TEST_FAILS(unequal("x", std::string("x")));
        UNIT_ASSERT_TEST_FAILS(unequal(1, 1));
        UNIT_ASSERT_TEST_FAILS(unequal(static_cast<unsigned short>(42), 42ul));

        UNIT_ASSERT_UNEQUAL("abc", std::string("cba"));
        UNIT_ASSERT_UNEQUAL(12l, 10);
        UNIT_ASSERT_UNEQUAL(33, 50);
    }

    Y_UNIT_TEST(LessThan) {
        auto lt = [](auto v1, auto v2) {
            UNIT_ASSERT_LT(v1, v2);
        };

        // less than
        UNIT_ASSERT_LT(std::string_view("1"), "2");
        UNIT_ASSERT_LT("2", std::string("3"));
        UNIT_ASSERT_LT("abc", std::string("azz"));
        UNIT_ASSERT_LT(2, 4);
        UNIT_ASSERT_LT(42ul, static_cast<unsigned short>(48));

        // equals
        UNIT_ASSERT_TEST_FAILS(lt(std::string_view("2"), "2"));
        UNIT_ASSERT_TEST_FAILS(lt("2", std::string("2")));
        UNIT_ASSERT_TEST_FAILS(lt("abc", std::string("abc")));
        UNIT_ASSERT_TEST_FAILS(lt(2, 2));
        UNIT_ASSERT_TEST_FAILS(lt(42ul, static_cast<unsigned short>(42)));

        // greater than
        UNIT_ASSERT_TEST_FAILS(lt(std::string_view("2"), "1"));
        UNIT_ASSERT_TEST_FAILS(lt("3", std::string("2")));
        UNIT_ASSERT_TEST_FAILS(lt("azz", std::string("abc")));
        UNIT_ASSERT_TEST_FAILS(lt(5, 2));
        UNIT_ASSERT_TEST_FAILS(lt(100ul, static_cast<unsigned short>(42)));
    }

    Y_UNIT_TEST(LessOrEqual) {
        auto le = [](auto v1, auto v2) {
            UNIT_ASSERT_LE(v1, v2);
        };

        // less than
        UNIT_ASSERT_LE(std::string_view("1"), "2");
        UNIT_ASSERT_LE("2", std::string("3"));
        UNIT_ASSERT_LE("abc", std::string("azz"));
        UNIT_ASSERT_LE(2, 4);
        UNIT_ASSERT_LE(42ul, static_cast<unsigned short>(48));

        // equals
        UNIT_ASSERT_LE(std::string_view("2"), "2");
        UNIT_ASSERT_LE("2", std::string("2"));
        UNIT_ASSERT_LE("abc", std::string("abc"));
        UNIT_ASSERT_LE(2, 2);
        UNIT_ASSERT_LE(42ul, static_cast<unsigned short>(42));

        // greater than
        UNIT_ASSERT_TEST_FAILS(le(std::string_view("2"), "1"));
        UNIT_ASSERT_TEST_FAILS(le("3", std::string("2")));
        UNIT_ASSERT_TEST_FAILS(le("azz", std::string("abc")));
        UNIT_ASSERT_TEST_FAILS(le(5, 2));
        UNIT_ASSERT_TEST_FAILS(le(100ul, static_cast<unsigned short>(42)));
    }

    Y_UNIT_TEST(GreaterThan) {
        auto gt = [](auto v1, auto v2) {
            UNIT_ASSERT_GT(v1, v2);
        };

        // less than
        UNIT_ASSERT_TEST_FAILS(gt(std::string_view("1"), "2"));
        UNIT_ASSERT_TEST_FAILS(gt("2", std::string("3")));
        UNIT_ASSERT_TEST_FAILS(gt("abc", std::string("azz")));
        UNIT_ASSERT_TEST_FAILS(gt(2, 4));
        UNIT_ASSERT_TEST_FAILS(gt(42ul, static_cast<unsigned short>(48)));

        // equals
        UNIT_ASSERT_TEST_FAILS(gt(std::string_view("2"), "2"));
        UNIT_ASSERT_TEST_FAILS(gt("2", std::string("2")));
        UNIT_ASSERT_TEST_FAILS(gt("abc", std::string("abc")));
        UNIT_ASSERT_TEST_FAILS(gt(2, 2));
        UNIT_ASSERT_TEST_FAILS(gt(42ul, static_cast<unsigned short>(42)));

        // greater than
        UNIT_ASSERT_GT(std::string_view("2"), "1");
        UNIT_ASSERT_GT("3", std::string("2"));
        UNIT_ASSERT_GT("azz", std::string("abc"));
        UNIT_ASSERT_GT(5, 2);
        UNIT_ASSERT_GT(100ul, static_cast<unsigned short>(42));
    }

    Y_UNIT_TEST(GreaterOrEqual) {
        auto ge = [](auto v1, auto v2) {
            UNIT_ASSERT_GE(v1, v2);
        };

        // less than
        UNIT_ASSERT_TEST_FAILS(ge(std::string_view("1"), "2"));
        UNIT_ASSERT_TEST_FAILS(ge("2", std::string("3")));
        UNIT_ASSERT_TEST_FAILS(ge("abc", std::string("azz")));
        UNIT_ASSERT_TEST_FAILS(ge(2, 4));
        UNIT_ASSERT_TEST_FAILS(ge(42ul, static_cast<unsigned short>(48)));

        // equals
        UNIT_ASSERT_GE(std::string_view("2"), "2");
        UNIT_ASSERT_GE("2", std::string("2"));
        UNIT_ASSERT_GE("abc", std::string("abc"));
        UNIT_ASSERT_GE(2, 2);
        UNIT_ASSERT_GE(42ul, static_cast<unsigned short>(42));

        // greater than
        UNIT_ASSERT_GE(std::string_view("2"), "1");
        UNIT_ASSERT_GE("3", std::string("2"));
        UNIT_ASSERT_GE("azz", std::string("abc"));
        UNIT_ASSERT_GE(5, 2);
        UNIT_ASSERT_GE(100ul, static_cast<unsigned short>(42));
    }

    Y_UNIT_TEST(ValuesEqual) {
        auto valuesEqual = [](auto v1, auto v2) {
            UNIT_ASSERT_VALUES_EQUAL(v1, v2);
        };
        UNIT_ASSERT_TEST_FAILS(valuesEqual(1, 2));
        UNIT_ASSERT_TEST_FAILS(valuesEqual(1l, static_cast<short>(2)));

        UNIT_ASSERT_VALUES_EQUAL("yandex", std::string("yandex"));
        UNIT_ASSERT_VALUES_EQUAL(1.0, 1.0);
    }

    Y_UNIT_TEST(ValuesUnequal) {
        auto valuesUnequal = [](auto v1, auto v2) {
            UNIT_ASSERT_VALUES_UNEQUAL(v1, v2);
        };
        UNIT_ASSERT_TEST_FAILS(valuesUnequal(5, 5));
        UNIT_ASSERT_TEST_FAILS(valuesUnequal(static_cast<char>(5), 5l));
        std::string test("test");
        UNIT_ASSERT_TEST_FAILS(valuesUnequal("test", test.data()));

        UNIT_ASSERT_VALUES_UNEQUAL("UNIT_ASSERT_VALUES_UNEQUAL", "UNIT_ASSERT_VALUES_EQUAL");
        UNIT_ASSERT_VALUES_UNEQUAL(1.0, 1.1);
    }

    class TTestException: public yexception {
    public:
        TTestException(const std::string& text = "test exception", bool throwMe = true)
            : ThrowMe(throwMe)
        {
            *this << text;
        }

        virtual ~TTestException() = default;

        virtual void Throw() {
            if (ThrowMe) {
                throw *this;
            }
        }

        std::string ThrowStr() {
            if (ThrowMe) {
                throw *this;
            }

            return {};
        }

        void AssertNoException() {
            UNIT_ASSERT_NO_EXCEPTION(Throw());
        }

        void AssertNoExceptionRet() {
            const std::string res = UNIT_ASSERT_NO_EXCEPTION_RESULT(ThrowStr()); 
        }

        template <class TExpectedException>
        void AssertException() {
            UNIT_ASSERT_EXCEPTION(Throw(), TExpectedException);
        }

        template <class TExpectedException, class T>
        void AssertExceptionContains(const T& substr) {
            UNIT_ASSERT_EXCEPTION_CONTAINS(Throw(), TExpectedException, substr);
        }

        template <class TExpectedException, class P>
        void AssertExceptionSatisfies(const P& predicate) {
            UNIT_ASSERT_EXCEPTION_SATISFIES(Throw(), TExpectedException, predicate);
        }

        int GetValue() const {
            return 5;           // just some value for predicate testing
        }

        bool ThrowMe;
    };

    class TOtherTestException: public TTestException {
    public:
        using TTestException::TTestException;

        // Throws other type of exception
        void Throw() override {
            if (ThrowMe) {
                throw *this;
            }
        }
    };

    Y_UNIT_TEST(Exception) {
        UNIT_ASSERT_TEST_FAILS(TTestException("", false).AssertException<TTestException>());
        UNIT_ASSERT_TEST_FAILS(TTestException().AssertException<TOtherTestException>());

        UNIT_ASSERT_EXCEPTION(TOtherTestException().Throw(), TTestException);
        UNIT_ASSERT_EXCEPTION(TTestException().Throw(), TTestException);
    }

    Y_UNIT_TEST(ExceptionAssertionContainsOtherExceptionMessage) {
        NUnitTest::TUnitTestFailChecker checker;
        {
            auto guard = checker.InvokeGuard();
            TTestException("custom exception message").AssertException<TOtherTestException>();
        }
        UNIT_ASSERT(checker.Failed());
        UNIT_ASSERT_STRING_CONTAINS(checker.Msg(), "custom exception message");
    }

    Y_UNIT_TEST(NoException) {
        UNIT_ASSERT_TEST_FAILS(TTestException().AssertNoException());
        UNIT_ASSERT_TEST_FAILS(TTestException().AssertNoExceptionRet());

        UNIT_ASSERT_NO_EXCEPTION(TTestException("", false).Throw());
    }

    Y_UNIT_TEST(ExceptionContains) {
        UNIT_ASSERT_TEST_FAILS(TTestException("abc").AssertExceptionContains<TTestException>("cba"));
        UNIT_ASSERT_TEST_FAILS(TTestException("abc").AssertExceptionContains<TTestException>(std::string_view("cba")));
        UNIT_ASSERT_TEST_FAILS(TTestException("abc").AssertExceptionContains<TTestException>(std::string("cba")));
        UNIT_ASSERT_TEST_FAILS(TTestException("abc").AssertExceptionContains<TTestException>(TYdbStringBuilder() << "cba"));

        UNIT_ASSERT_TEST_FAILS(TTestException("abc", false).AssertExceptionContains<TTestException>("bc"));

        UNIT_ASSERT_TEST_FAILS(TTestException("abc").AssertExceptionContains<TOtherTestException>("b"));

        UNIT_ASSERT_EXCEPTION_CONTAINS(TTestException("abc").Throw(), TTestException, "a");
    }

    Y_UNIT_TEST(ExceptionSatisfies) {
        const auto goodPredicate = [](const TTestException& e) { return e.GetValue() == 5; };
        const auto badPredicate  = [](const TTestException& e) { return e.GetValue() != 5; };
        UNIT_ASSERT_NO_EXCEPTION(TTestException().AssertExceptionSatisfies<TTestException>(goodPredicate));
        UNIT_ASSERT_TEST_FAILS(TTestException().AssertExceptionSatisfies<TTestException>(badPredicate));
        UNIT_ASSERT_TEST_FAILS(TTestException().AssertExceptionSatisfies<TOtherTestException>(goodPredicate));
    }
}
