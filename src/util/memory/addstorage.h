#pragma once

#include <src/util/system/align.h>

#include <ydb-cpp-sdk/util/system/defaults.h>

#include <cstddef>
#include <new>

namespace NPrivate {
    class TAdditionalStorageInfo {
    public:
        constexpr TAdditionalStorageInfo(std::size_t length) noexcept
            : Length_(length)
        {
        }

        constexpr std::size_t Length() const noexcept {
            return Length_;
        }

    private:
        std::size_t Length_;
    };
} // NPrivate

template <class T>
class alignas(::NPrivate::TAdditionalStorageInfo) TAdditionalStorage {
    using TInfo = ::NPrivate::TAdditionalStorageInfo;

public:
    TAdditionalStorage() noexcept = default;

    ~TAdditionalStorage() = default;

    void* operator new(std::size_t len1, std::size_t len2) {
        static_assert(alignof(T) >= alignof(TInfo));
        Y_ASSERT(len1 == sizeof(T));
        void* data = ::operator new(CombinedSizeOfInstanceWithTInfo() + len2);
        void* info = InfoPtr(static_cast<T*>(data));
        Y_UNUSED(new (info) TInfo(len2));

        return data;
    }

    void operator delete(void* ptr) noexcept {
        DoDelete(ptr);
    }

    void operator delete(void* ptr, std::size_t) noexcept {
        DoDelete(ptr);
    }

    void operator delete(void* ptr, std::size_t, std::size_t) noexcept {
        /*
         * this delete operator can be called automagically by compiler
         */

        DoDelete(ptr);
    }

    void* AdditionalData() const noexcept {
        return reinterpret_cast<char*>(const_cast<T*>(static_cast<const T*>(this)))
                + CombinedSizeOfInstanceWithTInfo();
    }

    static T* ObjectFromData(void* data) noexcept {
        return reinterpret_cast<T*>(static_cast<char*>(data) - CombinedSizeOfInstanceWithTInfo());
    }

    std::size_t AdditionalDataLength() const noexcept {
        return InfoPtr(static_cast<const T*>(this))->Length();
    }

private:
    static void DoDelete(void* ptr) noexcept {
        TInfo* info = InfoPtr(static_cast<T*>(ptr));
        info->~TInfo();
        ::operator delete(ptr);
    }

    static constexpr std::size_t CombinedSizeOfInstanceWithTInfo() noexcept {
        return AlignUp(InfoPtrOffset() + sizeof(TInfo), __STDCPP_DEFAULT_NEW_ALIGNMENT__);
    }

    static constexpr std::size_t InfoPtrOffset() noexcept {
        return AlignUp(sizeof(T), alignof(TInfo));
    }

    static constexpr TInfo* InfoPtr(T* instance) noexcept {
        return const_cast<TInfo*>(InfoPtr(static_cast<const T*>(instance)));
    }

    static constexpr const TInfo* InfoPtr(const T* instance) noexcept {
        return reinterpret_cast<const TInfo*>(
                reinterpret_cast<const char*>(instance) + InfoPtrOffset());
    }

private:
    void* operator new(std::size_t) = delete;
};
