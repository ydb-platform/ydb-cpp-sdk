#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/fwd.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/util/generic/fwd.h>
#include <src/util/generic/ptr.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

class TGetOpt {
public:
    class TIterator {
        friend class TGetOpt;

    public:
        char Key() const noexcept;
        const char* Arg() const noexcept;

        inline bool HaveArg() const noexcept {
            return Arg();
        }

        inline void operator++() {
            Next();
        }

        inline bool operator==(const TIterator& r) const noexcept {
            return AtEnd() == r.AtEnd();
        }

        inline bool operator!=(const TIterator& r) const noexcept {
            return !(*this == r);
        }

        inline TIterator& operator*() noexcept {
            return *this;
        }

        inline const TIterator& operator*() const noexcept {
            return *this;
        }

        inline TIterator* operator->() noexcept {
            return this;
        }

        inline const TIterator* operator->() const noexcept {
            return this;
        }

    private:
        TIterator() noexcept;
        TIterator(const TGetOpt* parent);

        void Next();
        bool AtEnd() const noexcept;

    private:
        class TIterImpl;
        TSimpleIntrusivePtr<TIterImpl> Impl_;
    };

    TGetOpt(int argc, const char* const* argv, const std::string& format);

    inline TIterator Begin() const {
        return TIterator(this);
    }

    inline TIterator End() const noexcept {
        return TIterator();
    }

private:
    class TImpl;
    TSimpleIntrusivePtr<TImpl> Impl_;
};
