#pragma once

#include <util/generic/string.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace testing {
    /**
     * When matching `const std::string_view&`, implicitly convert other strings and string views to `Eq` matchers.
     */
    template <typename T, typename TT>
    class Matcher<const std::basic_string_view<T, TT>&>: public internal::MatcherBase<const std::basic_string_view<T, TT>&> {
    public:
        Matcher() {
        }

        explicit Matcher(const MatcherInterface<const std::basic_string_view<T, TT>&>* impl)
            : internal::MatcherBase<const std::basic_string_view<T, TT>&>(impl) {
        }

        template <typename M, typename = typename std::remove_reference<M>::type::is_gtest_matcher>
        Matcher(M&& m)
            : internal::MatcherBase<const std::basic_string_view<T, TT>&>(std::forward<M>(m)) {
        }

        Matcher(const TBasicString<T, TT>& s) {
            *this = Eq(std::basic_string_view<T, TT>(s));
        }

        Matcher(const T* s) {
            *this = Eq(std::basic_string_view<T, TT>(s));
        }

        Matcher(std::basic_string_view<T, TT> s) {
            *this = Eq(s);
        }
    };

    /**
     * When matching `TBasicBuf`, implicitly convert other strings and string views to `Eq` matchers.
     */
    template <typename T, typename TT>
    class Matcher<std::basic_string_view<T, TT>>: public internal::MatcherBase<std::basic_string_view<T, TT>> {
    public:
        Matcher() {
        }

        explicit Matcher(const MatcherInterface <std::basic_string_view<T, TT>>* impl)
            : internal::MatcherBase<std::basic_string_view<T, TT>>(impl) {
        }

        explicit Matcher(const MatcherInterface<const std::basic_string_view<T, TT>&>* impl)
            : internal::MatcherBase<std::basic_string_view<T, TT>>(impl) {
        }

        template <typename M, typename = typename std::remove_reference<M>::type::is_gtest_matcher>
        Matcher(M&& m)
            : internal::MatcherBase<std::basic_string_view<T, TT>>(std::forward<M>(m)) {
        }

        Matcher(const TBasicString<T, TT>& s) {
            *this = Eq(TBasicString<T, TT>(s));
        }

        Matcher(const T* s) {
            *this = Eq(TBasicString<T, TT>(s));
        }

        Matcher(std::basic_string_view<T, TT> s) {
            *this = Eq(s);
        }
    };

    /**
     * When matching `const std::string&`, implicitly convert other strings and string views to `Eq` matchers.
     */
    template <typename T, typename TT>
    class Matcher<const TBasicString<T, TT>&>: public internal::MatcherBase<const TBasicString<T, TT>&> {
    public:
        Matcher() {
        }

        explicit Matcher(const MatcherInterface<const TBasicString<T, TT>&>* impl)
            : internal::MatcherBase<const TBasicString<T, TT>&>(impl) {
        }

        Matcher(const TBasicString<T, TT>& s) {
            *this = Eq(s);
        }

        template <typename M, typename = typename std::remove_reference<M>::type::is_gtest_matcher>
        Matcher(M&& m)
            : internal::MatcherBase<const TBasicString<T, TT>&>(std::forward<M>(m)) {
        }

        Matcher(const T* s) {
            *this = Eq(TBasicString<T, TT>(s));
        }
    };

    /**
     * When matching `std::string`, implicitly convert other strings and string views to `Eq` matchers.
     */
    template <typename T, typename TT>
    class Matcher<TBasicString<T, TT>>: public internal::MatcherBase<TBasicString<T, TT>> {
    public:
        Matcher() {
        }

        explicit Matcher(const MatcherInterface <TBasicString<T, TT>>* impl)
            : internal::MatcherBase<TBasicString<T, TT>>(impl) {
        }

        explicit Matcher(const MatcherInterface<const TBasicString<T, TT>&>* impl)
            : internal::MatcherBase<TBasicString<T, TT>>(impl) {
        }

        template <typename M, typename = typename std::remove_reference<M>::type::is_gtest_matcher>
        Matcher(M&& m)
            : internal::MatcherBase<TBasicString<T, TT>>(std::forward<M>(m)) {
        }

        Matcher(const TBasicString<T, TT>& s) {
            *this = Eq(s);
        }

        Matcher(const T* s) {
            *this = Eq(TBasicString<T, TT>(s));
        }
    };
}
