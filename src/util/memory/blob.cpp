#include <ydb-cpp-sdk/util/memory/blob.h>
#include "addstorage.h"

#include <ydb-cpp-sdk/util/system/yassert.h>
#include <src/util/system/filemap.h>
#include <src/util/system/mlock.h>
#include <src/util/stream/buffer.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <src/util/generic/buffer.h>
#include <ydb-cpp-sdk/util/generic/ylimits.h>
#include <ydb-cpp-sdk/util/generic/singleton.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>

template <class TCounter>
class TDynamicBlobBase: public TBlob::TBase,
                        public TRefCounted<TDynamicBlobBase<TCounter>, TCounter>,
                        public TAdditionalStorage<TDynamicBlobBase<TCounter>> {
    using TRefBase = TRefCounted<TDynamicBlobBase, TCounter>;

public:
    inline TDynamicBlobBase() = default;

    ~TDynamicBlobBase() override = default;

    void Ref() noexcept override {
        TRefBase::Ref();
    }

    void UnRef() noexcept override {
        TRefBase::UnRef();
    }

    inline void* Data() const noexcept {
        return this->AdditionalData();
    }

    inline size_t Length() const noexcept {
        return this->AdditionalDataLength();
    }
};

template <class TCounter>
class TBufferBlobBase: public TBlob::TBase, public TRefCounted<TBufferBlobBase<TCounter>, TCounter> {
    using TRefBase = TRefCounted<TBufferBlobBase, TCounter>;

public:
    inline TBufferBlobBase(TBuffer& buf) {
        Buf_.Swap(buf);
    }

    ~TBufferBlobBase() override = default;

    void Ref() noexcept override {
        TRefBase::Ref();
    }

    void UnRef() noexcept override {
        TRefBase::UnRef();
    }

    inline const TBuffer& Buffer() const noexcept {
        return Buf_;
    }

private:
    TBuffer Buf_;
};

template <class TCounter>
class TStringBlobBase: public TBlob::TBase, public TRefCounted<TStringBlobBase<TCounter>, TCounter> {
    using TRefBase = TRefCounted<TStringBlobBase, TCounter>;

public:
    inline TStringBlobBase(const std::string& s)
        : S_(s)
    {
    }

    TStringBlobBase(std::string&& s) noexcept
        : S_(std::move(s))
    {
    }

    ~TStringBlobBase() override = default;

    void Ref() noexcept override {
        TRefBase::Ref();
    }

    void UnRef() noexcept override {
        TRefBase::UnRef();
    }

    inline const std::string& String() const noexcept {
        return S_;
    }

private:
    const std::string S_;
};

template <class TCounter>
class TMappedBlobBase: public TBlob::TBase, public TRefCounted<TMappedBlobBase<TCounter>, TCounter> {
    using TRefBase = TRefCounted<TMappedBlobBase<TCounter>, TCounter>;

public:
    inline TMappedBlobBase(const TMemoryMap& map, ui64 offset, size_t len, EMappingMode mode)
        : Map_(map)
        , Mode_(mode)
    {
        Y_ENSURE(Map_.IsOpen(), std::string_view("memory map not open"));

        Map_.Map(offset, len);

        if (len && !Map_.Ptr()) { // Ptr is 0 for blob of size 0
            ythrow yexception() << "can not map(" << offset << ", " << len << ")";
        }

        if (Mode_ == EMappingMode::Locked) {
            LockMemory(Data(), Length());
        }
    }

    ~TMappedBlobBase() override {
        if (Mode_ == EMappingMode::Locked && Length()) {
            UnlockMemory(Data(), Length());
        }
    }

    void Ref() noexcept override {
        TRefBase::Ref();
    }

    void UnRef() noexcept override {
        TRefBase::UnRef();
    }

    inline const void* Data() const noexcept {
        return Map_.Ptr();
    }

    inline size_t Length() const noexcept {
        return Map_.MappedSize();
    }

private:
    TFileMap Map_;
    EMappingMode Mode_;
};

TBlob TBlob::SubBlob(size_t len) const {
    /*
     * may be slightly optimized
     */

    return SubBlob(0, len);
}

TBlob TBlob::SubBlob(size_t begin, size_t end) const {
    if (begin > Length() || end > Length() || begin > end) {
        ythrow yexception() << "incorrect subblob (" << begin << ", " << end << ", outer length = " << Length() << ")";
    }

    return TBlob(Begin() + begin, end - begin, S_.Base);
}

TBlob TBlob::DeepCopy() const {
    return TBlob::Copy(Data(), Length());
}

template <class TCounter>
static inline TBlob CopyConstruct(const void* data, size_t len) {
    using Base = TDynamicBlobBase<TCounter>;
    std::unique_ptr<Base> base(new (len) Base);

    Y_ASSERT(base->Length() == len);

    memcpy(base->Data(), data, len);

    TBlob ret(base->Data(), len, base.get());
    Y_UNUSED(base.release());

    return ret;
}

TBlob TBlob::CopySingleThreaded(const void* data, size_t length) {
    return CopyConstruct<TSimpleCounter>(data, length);
}

TBlob TBlob::Copy(const void* data, size_t length) {
    return CopyConstruct<TAtomicCounter>(data, length);
}

TBlob TBlob::NoCopy(const void* data, size_t length) {
    return TBlob(data, length, nullptr);
}

template <class TCounter>
static inline TBlob ConstructFromMap(const TMemoryMap& map, ui64 offset, size_t length, EMappingMode mode) {
    using TBase = TMappedBlobBase<TCounter>;
    std::unique_ptr<TBase> base(new TBase(map, offset, length, mode));
    TBlob ret(base->Data(), base->Length(), base.get());
    Y_UNUSED(base.release());

    return ret;
}

template <class TCounter, class T>
static inline TBlob ConstructAsMap(const T& t, EMappingMode mode) {
    TMemoryMap::EOpenMode openMode = (mode == EMappingMode::Precharged) ? (TMemoryMap::oRdOnly | TMemoryMap::oPrecharge) : TMemoryMap::oRdOnly;

    TMemoryMap map(t, openMode);
    const ui64 toMap = map.Length();

    if (toMap > Max<size_t>()) {
        ythrow yexception() << "can not map whole file(length = " << toMap << ")";
    }

    return ConstructFromMap<TCounter>(map, 0, static_cast<size_t>(toMap), mode);
}

TBlob TBlob::FromFileSingleThreaded(const std::string& path, EMappingMode mode) {
    return ConstructAsMap<TSimpleCounter>(path, mode);
}

TBlob TBlob::FromFile(const std::string& path, EMappingMode mode) {
    return ConstructAsMap<TAtomicCounter>(path, mode);
}

TBlob TBlob::FromFileSingleThreaded(const TFile& file, EMappingMode mode) {
    return ConstructAsMap<TSimpleCounter>(file, mode);
}

TBlob TBlob::FromFile(const TFile& file, EMappingMode mode) {
    return ConstructAsMap<TAtomicCounter>(file, mode);
}

TBlob TBlob::FromFileSingleThreaded(const std::string& path) {
    return ConstructAsMap<TSimpleCounter>(path, EMappingMode::Standard);
}

TBlob TBlob::FromFile(const std::string& path) {
    return ConstructAsMap<TAtomicCounter>(path, EMappingMode::Standard);
}

TBlob TBlob::FromFileSingleThreaded(const TFile& file) {
    return ConstructAsMap<TSimpleCounter>(file, EMappingMode::Standard);
}

TBlob TBlob::FromFile(const TFile& file) {
    return ConstructAsMap<TAtomicCounter>(file, EMappingMode::Standard);
}

TBlob TBlob::PrechargedFromFileSingleThreaded(const std::string& path) {
    return ConstructAsMap<TSimpleCounter>(path, EMappingMode::Precharged);
}

TBlob TBlob::PrechargedFromFile(const std::string& path) {
    return ConstructAsMap<TAtomicCounter>(path, EMappingMode::Precharged);
}

TBlob TBlob::PrechargedFromFileSingleThreaded(const TFile& file) {
    return ConstructAsMap<TSimpleCounter>(file, EMappingMode::Precharged);
}

TBlob TBlob::PrechargedFromFile(const TFile& file) {
    return ConstructAsMap<TAtomicCounter>(file, EMappingMode::Precharged);
}

TBlob TBlob::LockedFromFileSingleThreaded(const std::string& path) {
    return ConstructAsMap<TSimpleCounter>(path, EMappingMode::Locked);
}

TBlob TBlob::LockedFromFile(const std::string& path) {
    return ConstructAsMap<TAtomicCounter>(path, EMappingMode::Locked);
}

TBlob TBlob::LockedFromFileSingleThreaded(const TFile& file) {
    return ConstructAsMap<TSimpleCounter>(file, EMappingMode::Locked);
}

TBlob TBlob::LockedFromFile(const TFile& file) {
    return ConstructAsMap<TAtomicCounter>(file, EMappingMode::Locked);
}

TBlob TBlob::LockedFromMemoryMapSingleThreaded(const TMemoryMap& map, ui64 offset, size_t length) {
    return ConstructFromMap<TSimpleCounter>(map, offset, length, EMappingMode::Locked);
}

TBlob TBlob::LockedFromMemoryMap(const TMemoryMap& map, ui64 offset, size_t length) {
    return ConstructFromMap<TAtomicCounter>(map, offset, length, EMappingMode::Locked);
}

TBlob TBlob::FromMemoryMapSingleThreaded(const TMemoryMap& map, ui64 offset, size_t length) {
    return ConstructFromMap<TSimpleCounter>(map, offset, length, EMappingMode::Standard);
}

TBlob TBlob::FromMemoryMap(const TMemoryMap& map, ui64 offset, size_t length) {
    return ConstructFromMap<TAtomicCounter>(map, offset, length, EMappingMode::Standard);
}

template <class TCounter>
static inline TBlob ReadFromFile(const TFile& file, ui64 offset, size_t length) {
    using TBase = TDynamicBlobBase<TCounter>;
    std::unique_ptr<TBase> base(new (length) TBase);

    Y_ASSERT(base->Length() == length);

    file.Pload(base->Data(), length, offset);

    TBlob ret(base->Data(), length, base.get());
    Y_UNUSED(base.release());

    return ret;
}

template <class TCounter>
static inline TBlob ConstructFromFileContent(const TFile& file, ui64 offset, ui64 length) {
    if (length > Max<size_t>()) {
        ythrow yexception() << "can not read whole file(length = " << length << ")";
    }

    return ReadFromFile<TCounter>(file, offset, static_cast<size_t>(length));
}

TBlob TBlob::FromFileContentSingleThreaded(const std::string& path) {
    TFile file(path, RdOnly);
    return ConstructFromFileContent<TSimpleCounter>(file, 0, file.GetLength());
}

TBlob TBlob::FromFileContent(const std::string& path) {
    TFile file(path, RdOnly);
    return ConstructFromFileContent<TAtomicCounter>(file, 0, file.GetLength());
}

TBlob TBlob::FromFileContentSingleThreaded(const TFile& file) {
    return ConstructFromFileContent<TSimpleCounter>(file, 0, file.GetLength());
}

TBlob TBlob::FromFileContent(const TFile& file) {
    return ConstructFromFileContent<TAtomicCounter>(file, 0, file.GetLength());
}

TBlob TBlob::FromFileContentSingleThreaded(const TFile& file, ui64 offset, size_t length) {
    return ConstructFromFileContent<TSimpleCounter>(file, offset, length);
}

TBlob TBlob::FromFileContent(const TFile& file, ui64 offset, size_t length) {
    return ConstructFromFileContent<TAtomicCounter>(file, offset, length);
}

template <class TCounter>
static inline TBlob ConstructFromBuffer(TBuffer& in) {
    using TBase = TBufferBlobBase<TCounter>;
    std::unique_ptr<TBase> base(new TBase(in));

    TBlob ret(base->Buffer().Data(), base->Buffer().Size(), base.get());
    Y_UNUSED(base.release());

    return ret;
}

template <class TCounter>
static inline TBlob ConstructFromStream(IInputStream& in) {
    TBuffer buf;

    {
        TBufferOutput out(buf);

        TransferData(&in, &out);
    }

    return ConstructFromBuffer<TCounter>(buf);
}

TBlob TBlob::FromStreamSingleThreaded(IInputStream& in) {
    return ConstructFromStream<TSimpleCounter>(in);
}

TBlob TBlob::FromStream(IInputStream& in) {
    return ConstructFromStream<TAtomicCounter>(in);
}

TBlob TBlob::FromBufferSingleThreaded(TBuffer& in) {
    return ConstructFromBuffer<TSimpleCounter>(in);
}

TBlob TBlob::FromBuffer(TBuffer& in) {
    return ConstructFromBuffer<TAtomicCounter>(in);
}

template <class TCounter, class S>
TBlob ConstructFromString(S&& s) {
    using TBase = TStringBlobBase<TCounter>;
    auto base = MakeHolder<TBase>(std::forward<S>(s));

    TBlob ret(base->String().data(), base->String().size(), base.Get());
    Y_UNUSED(base.Release());

    return ret;
}

TBlob TBlob::FromStringSingleThreaded(const std::string& s) {
    return ConstructFromString<TSimpleCounter>(s);
}

TBlob TBlob::FromStringSingleThreaded(std::string&& s) {
    return ConstructFromString<TSimpleCounter>(std::move(s));
}

TBlob TBlob::FromString(const std::string& s) {
    return ConstructFromString<TAtomicCounter>(s);
}

TBlob TBlob::FromString(std::string&& s) {
    return ConstructFromString<TAtomicCounter>(std::move(s));
}
