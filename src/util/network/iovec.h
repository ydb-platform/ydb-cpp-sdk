#pragma once

#include <ydb-cpp-sdk/util/memory/tempbuf.h>
#include "ydb-cpp-sdk/util/network/init.h"
#include <ydb-cpp-sdk/util/system/yassert.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstring>
#include <limits>
#include <type_traits>


namespace NUtils::NIOVector {

    template <typename CSpan,
              auto CSpan::* CPtrToMemPtrBase, auto CSpan::* CPtrToMemLength,
              std::size_t CMaxCount = std::numeric_limits<std::size_t>::max()>
    struct TSpanAdapterTraits {
        static_assert(std::is_standard_layout_v<CSpan>,
                      "Underlying C struct must be a standard layout type.");

    private: // Type traits helpers

        template <typename U>
        using RemoveConstInsidePointer =
                std::add_pointer_t<
                    std::remove_const_t<
                        std::remove_pointer_t<U>>>;

        template <typename U, auto U::* M>
        static consteval auto GetTypeFromPtrToMem() {
            if constexpr (std::is_member_function_pointer_v<decltype(M)>) {
                return decltype((std::declval<U>().*M)()){};
            } else {
                return decltype(std::declval<U>().*M){};
            }
        }

        template <typename U, auto U::* M>
        using TypeFromPtrToMem = decltype(GetTypeFromPtrToMem<U, M>());

        template <typename U, auto U::* M>
        static consteval bool IsNothrowable() {
            if constexpr (std::is_member_function_pointer_v<decltype(M)>) {
                return noexcept((std::declval<U>().*M)());
            } else {
                return std::is_nothrow_copy_assignable_v<TypeFromPtrToMem<U, M>>;
            }
        }

    private: // Common traits of the underlying C struct

        struct Common {
            struct LowLevel {
                using Type = CSpan;
                using TPtrBase = TypeFromPtrToMem<Type, CPtrToMemPtrBase>;
                using TLength = TypeFromPtrToMem<Type, CPtrToMemLength>;

                static constexpr std::size_t MaxCount = CMaxCount;

                static_assert(std::is_pointer_v<TPtrBase>,
                              "Underlying C struct must have a pointer within it.");
                static_assert(std::is_integral_v<TLength>,
                              "Underlying C struct must have a size within it.");

                static_assert(!std::is_member_function_pointer_v<decltype(CPtrToMemPtrBase)>
                           && !std::is_member_function_pointer_v<decltype(CPtrToMemLength)>,
                           "Underlying C struct must have public non-static members.");

            public: // Getters/Setters

                static TPtrBase PtrBase(const Type& c_span) noexcept {
                    return c_span.*CPtrToMemPtrBase;
                }

                static TPtrBase& PtrBase(Type& c_span) noexcept {
                    return c_span.*CPtrToMemPtrBase;
                }


                static TLength Length(const Type& c_span) noexcept {
                    return c_span.*CPtrToMemLength;
                }

                static TLength& Length(Type& c_span) noexcept {
                    return c_span.*CPtrToMemLength;
                }
            };
        };

        static constexpr int UnknownInCompileTime = -1;

    public: // Traits of the high-level class

        template <typename CxxSpan,
                  auto CxxSpan::* CxxPtrToMemPtrBase, auto CxxSpan::* CxxPtrToMemLength,
                  int AreOffsetsCompatible = UnknownInCompileTime>
        struct With : public Common {
            using LowLevel = Common::LowLevel;

            struct HighLevel {
                using Type = CxxSpan;
                using TPtrBase = TypeFromPtrToMem<Type, CxxPtrToMemPtrBase>;
                using TLength = TypeFromPtrToMem<Type, CxxPtrToMemLength>;

                static_assert(std::is_pointer_v<TPtrBase>,
                              "High-level class must have a pointer within it.");
                static_assert(std::is_integral_v<TLength>,
                              "High-level class must have a size within it.");
            };

        private: // Helper methods for adapting incompatible classes

            static LowLevel::TPtrBase PtrBase(const HighLevel::Type& cxx_span)
                    noexcept(IsNothrowable<typename HighLevel::Type, CxxPtrToMemPtrBase>()) {
                
                using LPtr = RemoveConstInsidePointer<typename LowLevel::TPtrBase>;
                using HPtr = RemoveConstInsidePointer<typename HighLevel::TPtrBase>;

                static_assert(std::is_same_v<LPtr, HPtr>
                           || std::is_same_v<HPtr, void*>
                           || std::is_same_v<HPtr, char*>
                           || std::is_same_v<HPtr, unsigned char*>
                           || std::is_same_v<HPtr, std::byte*>,
                           "Cast between T* and V* may lead to UB, if it causes "
                           "a violation of the strict aliasing rules and "
                           "access will take place; see [basic.lval] par.11.");

                // NOTE: underlying C structs almost always use pointers to
                // a non-const data, even for reading with the guarantee that
                // this data will never be overwritten, so we have to remove
                // constness out of obtained pointers.

                if constexpr (std::is_member_function_pointer_v<decltype(CxxPtrToMemPtrBase)>) {
                    return reinterpret_cast<LPtr>(const_cast<HPtr>((cxx_span.*CxxPtrToMemPtrBase)()));
                } else {
                    return reinterpret_cast<LPtr>(const_cast<HPtr>(cxx_span.*CxxPtrToMemPtrBase));
                }
            }

            static LowLevel::TLength Length(const HighLevel::Type& cxx_span) 
                    noexcept(IsNothrowable<typename HighLevel::Type, CxxPtrToMemLength>()) {
                if constexpr (std::is_member_function_pointer_v<decltype(CxxPtrToMemLength)>) {
                    return static_cast<LowLevel::TLength>((cxx_span.*CxxPtrToMemLength)());
                } else {
                    return static_cast<LowLevel::TLength>(cxx_span.*CxxPtrToMemLength);
                }
            }

        public:
            static LowLevel::Type Make(const HighLevel::Type& cxx_span)
                    noexcept(noexcept(PtrBase(cxx_span)) && noexcept(Length(cxx_span))) {
                typename LowLevel::Type c_span;
                c_span.*CPtrToMemPtrBase = PtrBase(cxx_span);
                c_span.*CPtrToMemLength = Length(cxx_span);
                return c_span;
            }

        private: // Helpers constants and methods to check compatibility

            static constexpr bool IsSizeCompatible =
                    sizeof(typename LowLevel::Type) == sizeof(typename HighLevel::Type);
            static constexpr bool IsAlignCompatible =
                    alignof(typename LowLevel::Type) == alignof(typename HighLevel::Type);
            static constexpr bool IsPtrBaseSizeCompatible =
                    sizeof(typename LowLevel::TPtrBase) == sizeof(typename HighLevel::TPtrBase);
            static constexpr bool IsLengthSizeCompatible =
                    sizeof(typename LowLevel::TLength) == sizeof(typename HighLevel::TLength);
            static constexpr bool IsTriviallyCopyable =
                    std::is_trivially_copyable_v<typename LowLevel::Type>
                        && std::is_trivially_copyable_v<typename HighLevel::Type>;

            static constexpr bool AreCommonTraitsCompatible =
                    IsSizeCompatible && IsAlignCompatible
                        && IsPtrBaseSizeCompatible && IsLengthSizeCompatible
                        && IsTriviallyCopyable;

            static bool IsCompatibleImpl() {
                using HType = typename HighLevel::Type;
                using HPtr = typename HighLevel::TPtrBase;
                using HLen = typename HighLevel::TLength;
                using LType = typename LowLevel::Type;
                using LPtr = typename LowLevel::TPtrBase;
                using LLen = typename LowLevel::TLength;

                int data = 42;
                auto data_bytes = std::bit_cast<std::array<std::byte, sizeof(int)>>(data);
                
                LType c_span;
                LowLevel::PtrBase(c_span) = data_bytes.data();
                LowLevel::Length(c_span) = 0;

                const auto c_bytes = std::bit_cast<std::array<std::byte, sizeof(LType)>>(c_span);
                std::array<std::byte, sizeof(HType)> cxx_bytes;

                if constexpr (std::is_constructible_v<HType, HPtr, HLen>) {
                    HType cxx_span(reinterpret_cast<HPtr>(data_bytes.data()), 0);
                    cxx_bytes = std::bit_cast<std::array<std::byte, sizeof(HType)>>(cxx_span);
                } else if constexpr (std::is_constructible_v<HType, HLen, HPtr>) {
                    HType cxx_span(0, reinterpret_cast<HPtr>(data_bytes.data()));
                    cxx_bytes = std::bit_cast<std::array<std::byte, sizeof(HType)>>(cxx_span);
                } else {
                    return false;
                }

                // NOTE: this check will give predictable results on most platforms
                // except architectures with word addressing, segmentation memory.
                // In the worst case two pointers to the same object might have
                // different value representation, so two classes will be treated
                // as layout-incompatible, and that will prevent potentially
                // `memcpy` usage.
                if (std::memcmp(c_bytes.data(), cxx_bytes.data(), sizeof(LPtr)) == 0) {
                    return std::memcmp(c_bytes.data() + alignof(LPtr),
                                       cxx_bytes.data() + alignof(LPtr),
                                       sizeof(LLen)) == 0;
                } else if (std::memcmp(c_bytes.data(), cxx_bytes.data(), sizeof(LLen)) == 0) {
                    return std::memcmp(c_bytes.data() + alignof(LLen),
                                       cxx_bytes.data() + alignof(LLen),
                                       sizeof(LPtr)) == 0;
                }
                return false;
            }

        public:
            static constexpr bool IsCompatibilityKnownInCompileTime =
                    AreOffsetsCompatible != UnknownInCompileTime;

            static constexpr bool IsCompatibleByCompileTime =
                    IsCompatibilityKnownInCompileTime
                        && AreCommonTraitsCompatible
                        && static_cast<bool>(AreOffsetsCompatible);

            static inline const bool IsCompatibleByRuntime =
                    IsCompatibleByCompileTime
                        || (AreCommonTraitsCompatible && IsCompatibleImpl());
        };
    };

    template <typename LowLevel>
    class TContIOVectorBase {
        TTempArray<typename LowLevel::Type> Buffer_;
        LowLevel::Type* Current_;
        std::size_t Count_;

    public:
        explicit TContIOVectorBase(std::size_t count)
                : Buffer_(count)
                , Current_(new(Buffer_.Data()) LowLevel::Type[count])
                , Count_(count) {
        }

        TContIOVectorBase(const TContIOVectorBase&) = delete;
        TContIOVectorBase& operator=(const TContIOVectorBase&) = delete;

        TContIOVectorBase(TContIOVectorBase&&) noexcept = default;
        TContIOVectorBase& operator=(TContIOVectorBase&&) noexcept = default;

        ~TContIOVectorBase() = default;

    public: // Byte info and manipulation

        std::size_t Bytes() const noexcept {
            std::size_t acc = 0;
            for (auto iter = Current_, end = Current_ + Count_; iter != end; ++iter) {
                acc += LowLevel::Length(*iter);
            }
            return acc;
        }

        bool Complete() const noexcept {
            return Count_ == 0;
        }

        const LowLevel::Type* Current() const noexcept {
            return Current_;
        }

        std::size_t Count() const noexcept {
            return Count_;
        }

        void Proceed(std::size_t processed_bytes) {
            using L = LowLevel;

            for (; Count_ != 0; processed_bytes -= L::Length(*Current_), ++Current_, --Count_) {
                if (processed_bytes < L::Length(*Current_)) {
                    L::Length(*Current_) -= processed_bytes;
                    const auto next = reinterpret_cast<std::byte*>(L::PtrBase(*Current_))
                            + processed_bytes;
                    L::PtrBase(*Current_) = reinterpret_cast<L::TPtrBase>(next);
                    return;
                }
            }

            if (processed_bytes != 0) {
                Y_ASSERT(0 && "non zero length left");
            }
        }

    protected:
        template <typename TCompatibilityTraits>
        requires std::is_same_v<typename TCompatibilityTraits::LowLevel::Type,
                                typename LowLevel::Type>
        void Populate(const TCompatibilityTraits::HighLevel::Type* cxx_span, std::size_t count) {
            count = std::min(count, Count_);
            const auto bytes = count * sizeof(typename TCompatibilityTraits::LowLevel::Type);

            if constexpr (TCompatibilityTraits::IsCompatibilityKnownInCompileTime) {
                if constexpr (TCompatibilityTraits::IsCompatibleByCompileTime) {
                    std::memcpy(Current_, cxx_span, bytes);
                } else {
                    std::transform(cxx_span, cxx_span + count, Current_, TCompatibilityTraits::Make);
                }
            } else {
                if (TCompatibilityTraits::IsCompatibleByRuntime) {
                    std::memcpy(Current_, cxx_span, bytes);
                } else {
                    std::transform(cxx_span, cxx_span + count, Current_, TCompatibilityTraits::Make);
                }
            }
        }

        void Swap(TContIOVectorBase& rhs) noexcept {
            using std::swap;
            swap(Buffer_, rhs.Buffer);
            swap(Current_, rhs.Current_);
            swap(Count_, rhs.Count_);
        }
    };

    struct Result {
        ssize_t processed_bytes = 0;
        ssize_t error = 0;
    };

    template <typename TCompatibilityTraits, typename THandler>
    class TContIOVector : public TContIOVectorBase<typename TCompatibilityTraits::LowLevel> {
        using Base = TContIOVectorBase<typename TCompatibilityTraits::LowLevel>;

        static constexpr bool IsNothrowHandler = noexcept(THandler{}(0, nullptr, 0));

    public:
        TContIOVector(const TCompatibilityTraits::HighLevel::Type* cxx_span, std::size_t count)
                : Base(count) {
            Base::template Populate<TCompatibilityTraits>(cxx_span, count);
        }

    public: // Processing bytes
        
        ssize_t TryProcess(SOCKET socket) noexcept(IsNothrowHandler) {
            return THandler{}(socket, Base::Current(),
                              std::min(Base::Count(), TCompatibilityTraits::LowLevel::MaxCount));
        }

        Result TryProcessAllBytes(SOCKET socket) noexcept(IsNothrowHandler) {
            Result result;
            auto response = TryProcess(socket);
            if (response < 0) {
                result.error = response;
                return result;
            }

            result.processed_bytes = response;
            if (static_cast<std::size_t>(response) == Base::Bytes()) {
                return result;
            }

            while(Base::Proceed(static_cast<std::size_t>(response)), !Base::Complete()) {
                response = TryProcess(socket);
                if (response < 0) {
                    result.error = response;
                    return result;
                }
                result.processed_bytes += response;
            }
            return result;
        }
    };

} // namespace NUtils::NIOVector
