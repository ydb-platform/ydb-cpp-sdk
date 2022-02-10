#pragma once
#include "aligned_page_pool.h"
#include "mkql_mem_info.h"
#include <ydb/library/yql/public/udf/udf_allocator.h>
#include <ydb/library/yql/public/udf/udf_value.h>
#include <util/system/defaults.h>
#include <util/system/tls.h>
#include <util/system/align.h>
#include <new>
#include <unordered_map>

namespace NKikimr {

namespace NMiniKQL {

const ui64 MKQL_ALIGNMENT = 16;

struct TAllocPageHeader {
    ui64 Capacity;
    ui64 Offset;
    ui64 UseCount;
    ui64 Deallocated;
    ui64 Padding;
    TAllocPageHeader* Link;
};

constexpr ui32 MaxPageUserData = TAlignedPagePool::POOL_PAGE_SIZE - sizeof(TAllocPageHeader);

static_assert(sizeof(TAllocPageHeader) % MKQL_ALIGNMENT == 0, "Incorrect size of header");

struct TAllocState : public TAlignedPagePool
{
    struct TListEntry {
        TListEntry *Left = nullptr;
        TListEntry *Right = nullptr;

        void Link(TListEntry* root) noexcept;
        void Unlink() noexcept;
        void InitLinks() noexcept { Left = Right = this; }
    };

#ifndef NDEBUG
    std::unordered_map<TMemoryUsageInfo*, TIntrusivePtr<TMemoryUsageInfo>> ActiveMemInfo;
#endif
    bool SupportsSizedAllocators = false;

    void* LargeAlloc(size_t size) {
        return Alloc(size);
    }

    void LargeFree(void* ptr, size_t size) noexcept {
        Free(ptr, size);
    }

    static TAllocPageHeader EmptyPageHeader;
    TAllocPageHeader* CurrentPage = &EmptyPageHeader;
    TListEntry OffloadedBlocksRoot;

    ::NKikimr::NUdf::TBoxedValueLink Root;

    NKikimr::NUdf::TBoxedValueLink* GetRoot() noexcept {
        return &Root;
    }

    explicit TAllocState(const TAlignedPagePoolCounters& counters, bool supportsSizedAllocators);
    void KillAllBoxed();
    void InvalidateMemInfo();
    size_t GetDeallocatedInPages() const;
};

extern Y_POD_THREAD(TAllocState*) TlsAllocState;

class TScopedAlloc {
public:
    explicit TScopedAlloc(const TAlignedPagePoolCounters& counters = TAlignedPagePoolCounters(), bool supportsSizedAllocators = false)
        : MyState_(counters, supportsSizedAllocators)
    {
        Acquire();
    }

    ~TScopedAlloc()
    {
        MyState_.KillAllBoxed();
        Release();
    }

    TAllocState& Ref() {
        return MyState_;
    }

    void Acquire() {
        if (!AttachedCount_) {
            PrevState_ = TlsAllocState;
            TlsAllocState = &MyState_;
        } else {
            Y_VERIFY(TlsAllocState == &MyState_, "Mismatch allocator in thread");
        }

        ++AttachedCount_;
    }

    void Release() {
        if (AttachedCount_ && --AttachedCount_ == 0) {
            Y_VERIFY(TlsAllocState == &MyState_, "Mismatch allocator in thread");
            TlsAllocState = PrevState_;
            PrevState_ = nullptr;
        }
    }

    size_t GetUsed() const { return MyState_.GetUsed(); }
    size_t GetPeakUsed() const { return MyState_.GetPeakUsed(); } 
    size_t GetAllocated() const { return MyState_.GetAllocated(); }
    size_t GetPeakAllocated() const { return MyState_.GetPeakAllocated(); } 

    size_t GetLimit() const { return MyState_.GetLimit(); }
    void SetLimit(size_t limit) { MyState_.SetLimit(limit); }
    void DisableStrictAllocationCheck() { MyState_.DisableStrictAllocationCheck(); }

    void ReleaseFreePages() { MyState_.ReleaseFreePages(); }
    void InvalidateMemInfo() { MyState_.InvalidateMemInfo(); }

    bool IsAttached() const { return AttachedCount_ > 0; }

private:
    TAllocState MyState_;
    size_t AttachedCount_ = 0;
    TAllocState* PrevState_ = nullptr;
};

class TInjectedAlloc {
public:
    explicit TInjectedAlloc(const TAlignedPagePoolCounters& counters = TAlignedPagePoolCounters(), bool supportsSizedAllocators = false)
        : OldTlsAllocState(TlsAllocState)
    {
        TlsAllocState = nullptr;
        InjectedAlloc = std::make_shared<TScopedAlloc>(counters, supportsSizedAllocators);
        AcquireOriginal();
    }

    ~TInjectedAlloc() {
        AcquireOriginal();
    }

    TScopedAlloc& ScopedRef() {
        return *InjectedAlloc;
    }

    TAllocState& InjectedState() {
        return InjectedAlloc->Ref();
    }

    std::shared_ptr<TScopedAlloc> InjectedScopeAlloc() {
        return InjectedAlloc;
    }

    void AcquireOriginal() {
        InjectedAlloc->Release();
        TlsAllocState = OldTlsAllocState;
    }

    void AcquireInjected() {
        TlsAllocState = nullptr;
        InjectedAlloc->Acquire();
    }
private:
    TAllocState* OldTlsAllocState;
    std::shared_ptr<TScopedAlloc> InjectedAlloc;
};

class TInjectFreeGuard: TNonCopyable {
public:
    TInjectFreeGuard(TInjectedAlloc* injectedAlloc = nullptr)
        : InjectedAllocPtr(injectedAlloc)
    {
        if (InjectedAllocPtr) {
            InjectedAllocPtr->AcquireInjected();
        }
    }

    ~TInjectFreeGuard() {
        if (InjectedAllocPtr) {
            InjectedAllocPtr->AcquireOriginal();
        }
    }

    void Free() {
        if (InjectedAllocPtr) {
            InjectedAllocPtr->AcquireOriginal();
            InjectedAllocPtr = nullptr;
        }
    }
private:
    TInjectedAlloc* InjectedAllocPtr;
};

class TPagedArena {
public:
    TPagedArena(TAlignedPagePool* pagePool) noexcept
        : PagePool_(pagePool)
        , CurrentPage_(&TAllocState::EmptyPageHeader)
    {}

    TPagedArena(const TPagedArena&) = delete;
    TPagedArena(TPagedArena&& other) noexcept
        : PagePool_(other.PagePool_)
        , CurrentPage_(other.CurrentPage_)
    {
        other.CurrentPage_ = &TAllocState::EmptyPageHeader;
    }

    void operator=(const TPagedArena&) = delete;
    void operator=(TPagedArena&& other) noexcept {
        Clear();
        PagePool_ = other.PagePool_;
        CurrentPage_ = other.CurrentPage_;
        other.CurrentPage_ = &TAllocState::EmptyPageHeader;
    }

    ~TPagedArena() noexcept {
        Clear();
    }

    void* Alloc(size_t sz) {
        if (Y_LIKELY(CurrentPage_->Offset + sz <= CurrentPage_->Capacity)) {
            void* ret = (char*)CurrentPage_ + CurrentPage_->Offset;
            CurrentPage_->Offset = AlignUp(CurrentPage_->Offset + sz, MKQL_ALIGNMENT);
            return ret;
        }

        return AllocSlow(sz);
    }

    void Clear() noexcept;

private:
    void* AllocSlow(size_t sz);

private:
    TAlignedPagePool* PagePool_;
    TAllocPageHeader* CurrentPage_ = &TAllocState::EmptyPageHeader;
};

void* MKQLAllocSlow(size_t sz, TAllocState* state);
inline void* MKQLAllocFastDeprecated(size_t sz, TAllocState* state) {
    Y_VERIFY_DEBUG(state);
    auto currPage = state->CurrentPage;
    if (Y_LIKELY(currPage->Offset + sz <= currPage->Capacity)) {
        void* ret = (char*)currPage + currPage->Offset;
        currPage->Offset = AlignUp(currPage->Offset + sz, MKQL_ALIGNMENT);
        ++currPage->UseCount;
        return ret;
    }

    return MKQLAllocSlow(sz, state);
}

inline void* MKQLAllocFastWithSize(size_t sz, TAllocState* state) {
    Y_VERIFY_DEBUG(state);
    if (state->SupportsSizedAllocators && sz > MaxPageUserData) {
        state->OffloadAlloc(sizeof(TAllocState::TListEntry) + sz);
        auto ret = (TAllocState::TListEntry*)malloc(sizeof(TAllocState::TListEntry) + sz);
        ret->Link(&state->OffloadedBlocksRoot);
        return ret + 1;
    }

    auto currPage = state->CurrentPage;
    if (Y_LIKELY(currPage->Offset + sz <= currPage->Capacity)) {
        void* ret = (char*)currPage + currPage->Offset;
        currPage->Offset = AlignUp(currPage->Offset + sz, MKQL_ALIGNMENT);
        ++currPage->UseCount;
        return ret;
    }

    return MKQLAllocSlow(sz, state);
}

void MKQLFreeSlow(TAllocPageHeader* header) noexcept;

inline void MKQLFreeDeprecated(const void* p) noexcept {
    if (!p) {
        return;
    }

    TAllocPageHeader* header = (TAllocPageHeader*)TAllocState::GetPageStart(p);
    if (Y_LIKELY(--header->UseCount != 0)) {
        return;
    }

    MKQLFreeSlow(header);
}

inline void MKQLFreeFastWithSize(const void* mem, size_t sz, TAllocState* state) noexcept {
    if (!mem) {
        return;
    }

    Y_VERIFY_DEBUG(state);
    if (state->SupportsSizedAllocators && sz > MaxPageUserData) {
        auto entry = (TAllocState::TListEntry*)(mem) - 1;
        entry->Unlink();
        free(entry);
        state->OffloadFree(sizeof(TAllocState::TListEntry) + sz);
        return;
    }

    TAllocPageHeader* header = (TAllocPageHeader*)TAllocState::GetPageStart(mem);
    if (Y_LIKELY(--header->UseCount != 0)) {
        header->Deallocated += sz;
        return;
    }

    MKQLFreeSlow(header);
}

inline void* MKQLAllocDeprecated(size_t sz) {
    return MKQLAllocFastDeprecated(sz, TlsAllocState);
}

inline void* MKQLAllocWithSize(size_t sz) {
    return MKQLAllocFastWithSize(sz, TlsAllocState);
}

inline void MKQLFreeWithSize(const void* mem, size_t sz) noexcept {
    return MKQLFreeFastWithSize(mem, sz, TlsAllocState);
}

inline void MKQLRegisterObject(NUdf::TBoxedValue* value) noexcept {
    return value->Link(TlsAllocState->GetRoot());
}

inline void MKQLUnregisterObject(NUdf::TBoxedValue* value) noexcept {
    return value->Unlink();
}

struct TWithMiniKQLAlloc {
    void* operator new(size_t sz) {
        return MKQLAllocWithSize(sz);
    }

    void* operator new[](size_t sz) {
        return MKQLAllocWithSize(sz);
    }

    void operator delete(void *mem, std::size_t sz) noexcept {
        MKQLFreeWithSize(mem, sz);
    }

    void operator delete[](void *mem, std::size_t sz) noexcept {
        MKQLFreeWithSize(mem, sz);
    }
};

template <typename T, typename... Args>
T* AllocateOn(TAllocState* state, Args&&... args)
{
    return ::new(MKQLAllocFastWithSize(sizeof(T), state)) T(std::forward<Args>(args)...);
    static_assert(std::is_base_of<TWithMiniKQLAlloc, T>::value, "Class must inherit TWithMiniKQLAlloc.");
}

template <typename Type>
struct TMKQLAllocator
{
    typedef Type value_type;
    typedef Type* pointer;
    typedef const Type* const_pointer;
    typedef Type& reference;
    typedef const Type& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    TMKQLAllocator() noexcept = default;
    ~TMKQLAllocator() noexcept = default;

    template<typename U> TMKQLAllocator(const TMKQLAllocator<U>&) noexcept {}
    template<typename U> struct rebind { typedef TMKQLAllocator<U> other; };
    template<typename U> bool operator==(const TMKQLAllocator<U>&) const { return true; }
    template<typename U> bool operator!=(const TMKQLAllocator<U>&) const { return false; }

    static pointer allocate(size_type n, const void* = nullptr)
    {
        return static_cast<pointer>(MKQLAllocWithSize(n * sizeof(value_type)));
    }

    static void deallocate(const_pointer p, size_type n) noexcept
    {
        return MKQLFreeWithSize(p, n * sizeof(value_type));
    }
};

template <typename T>
class TPagedList
{
public:
    static_assert(sizeof(T) <= TAlignedPagePool::POOL_PAGE_SIZE, "Too big object");
    static constexpr size_t OBJECTS_PER_PAGE = TAlignedPagePool::POOL_PAGE_SIZE / sizeof(T);

    class TIterator;
    class TConstIterator;

    TPagedList(TAlignedPagePool& pool)
        : Pool(pool)
        , IndexInLastPage(OBJECTS_PER_PAGE)
    {}

    TPagedList(const TPagedList&) = delete;
    TPagedList(TPagedList&&) = delete;

    ~TPagedList() {
        Clear();
    }

    void Add(T&& value) {
        if (IndexInLastPage < OBJECTS_PER_PAGE) {
            auto ptr = ObjectAt(Pages.back(), IndexInLastPage);
            new(ptr) T(std::move(value));
            ++IndexInLastPage;
            return;
        }

        auto ptr = Pool.GetPage();
        IndexInLastPage = 1;
        Pages.push_back(ptr);
        new(ptr) T(std::move(value));
    }

    void Clear() {
        for (ui32 i = 0; i + 1 < Pages.size(); ++i) {
            for (ui32 objIndex = 0; objIndex < OBJECTS_PER_PAGE; ++objIndex) {
                ObjectAt(Pages[i], objIndex)->~T();
            }

            Pool.ReturnPage(Pages[i]);
        }

        if (!Pages.empty()) {
            for (ui32 objIndex = 0; objIndex < IndexInLastPage; ++objIndex) {
                ObjectAt(Pages.back(), objIndex)->~T();
            }

            Pool.ReturnPage(Pages.back());
        }

        TPages().swap(Pages);
        IndexInLastPage = OBJECTS_PER_PAGE;
    }

    const T& operator[](size_t i) const {
        const auto table = i / OBJECTS_PER_PAGE;
        const auto index = i % OBJECTS_PER_PAGE;
        return *ObjectAt(Pages[table], index);
    }

    size_t Size() const {
        return Pages.empty() ? 0 : ((Pages.size() - 1) * OBJECTS_PER_PAGE + IndexInLastPage);
    }

    TConstIterator Begin() const {
        return TConstIterator(this, 0, 0);
    }

    TConstIterator begin() const {
        return Begin();
    }

    TConstIterator End() const {
        if (IndexInLastPage == OBJECTS_PER_PAGE) {
            return TConstIterator(this, Pages.size(), 0);
        }

        return TConstIterator(this, Pages.size() - 1, IndexInLastPage);
    }

    TConstIterator end() const {
        return End();
    }

    TIterator Begin() {
        return TIterator(this, 0, 0);
    }

    TIterator begin() {
        return Begin();
    }

    TIterator End() {
        if (IndexInLastPage == OBJECTS_PER_PAGE) {
            return TIterator(this, Pages.size(), 0);
        }

        return TIterator(this, Pages.size() - 1, IndexInLastPage);
    }

    TIterator end() {
        return End();
    }

    class TIterator
    {
    public:
        using TOwner = TPagedList<T>;

        TIterator()
            : Owner(nullptr)
            , PageNo(0)
            , PageIndex(0)
        {}

        TIterator(const TIterator&) = default;
        TIterator& operator=(const TIterator&) = default;

        TIterator(TOwner* owner, size_t pageNo, size_t pageIndex)
            : Owner(owner)
            , PageNo(pageNo)
            , PageIndex(pageIndex)
        {}

        T& operator*() {
            Y_VERIFY_DEBUG(PageIndex < OBJECTS_PER_PAGE);
            Y_VERIFY_DEBUG(PageNo < Owner->Pages.size());
            Y_VERIFY_DEBUG(PageNo + 1 < Owner->Pages.size() || PageIndex < Owner->IndexInLastPage);
            return *Owner->ObjectAt(Owner->Pages[PageNo], PageIndex);
        }

        TIterator& operator++() {
            if (++PageIndex == OBJECTS_PER_PAGE) {
                ++PageNo;
                PageIndex = 0;
            }

            return *this;
        }

        bool operator==(const TIterator& other) const {
            return PageNo == other.PageNo && PageIndex == other.PageIndex;
        }

        bool operator!=(const TIterator& other) const {
            return !operator==(other);
        }

    private:
        TOwner* Owner;
        size_t PageNo;
        size_t PageIndex;
    };

    class TConstIterator
    {
    public:
        using TOwner = TPagedList<T>;

        TConstIterator()
            : Owner(nullptr)
            , PageNo(0)
            , PageIndex(0)
        {}

        TConstIterator(const TConstIterator&) = default;
        TConstIterator& operator=(const TConstIterator&) = default;

        TConstIterator(const TOwner* owner, size_t pageNo, size_t pageIndex)
            : Owner(owner)
            , PageNo(pageNo)
            , PageIndex(pageIndex)
        {}

        const T& operator*() {
            Y_VERIFY_DEBUG(PageIndex < OBJECTS_PER_PAGE);
            Y_VERIFY_DEBUG(PageNo < Owner->Pages.size());
            Y_VERIFY_DEBUG(PageNo + 1 < Owner->Pages.size() || PageIndex < Owner->IndexInLastPage);
            return *Owner->ObjectAt(Owner->Pages[PageNo], PageIndex);
        }

        TConstIterator& operator++() {
            if (++PageIndex == OBJECTS_PER_PAGE) {
                ++PageNo;
                PageIndex = 0;
            }

            return *this;
        }

        bool operator==(const TConstIterator& other) const {
            return PageNo == other.PageNo && PageIndex == other.PageIndex;
        }

        bool operator!=(const TConstIterator& other) const {
            return !operator==(other);
        }

    private:
        const TOwner* Owner;
        size_t PageNo;
        size_t PageIndex;
    };

private:
    static const T* ObjectAt(const void* page, size_t objectIndex) {
        return reinterpret_cast<const T*>(static_cast<const char*>(page) + objectIndex * sizeof(T));
    }

    static T* ObjectAt(void* page, size_t objectIndex) {
        return reinterpret_cast<T*>(static_cast<char*>(page) + objectIndex * sizeof(T));
    }

    TAlignedPagePool& Pool;
    using TPages = std::vector<void*, TMKQLAllocator<void*>>;
    TPages Pages;
    size_t IndexInLastPage;
};


} // NMiniKQL

} // NKikimr
