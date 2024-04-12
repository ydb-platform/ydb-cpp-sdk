#include <ydb-cpp-sdk/library/yson/node/node.h>

#include <ydb-cpp-sdk/library/yson/node/node_io.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/library/yson/writer.h>

#include <src/util/generic/overloaded.h>
#include <ydb-cpp-sdk/util/string/escape.h>
=======
#include <src/library/yson/writer.h>

#include <src/util/generic/overloaded.h>
#include <src/util/string/escape.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#include <iostream>

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

bool TNode::TNull::operator==(const TNull&) const {
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool TNode::TUndefined::operator==(const TUndefined&) const {
    return true;
}

////////////////////////////////////////////////////////////////////////////////

namespace NNodeCmp {

bool IsComparableType(const TNode::EType type) {
    switch (type) {
        case TNode::String:
        case TNode::Int64:
        case TNode::Uint64:
        case TNode::Double:
        case TNode::Bool:
        case TNode::Null:
        case TNode::Undefined:
            return true;
        default:
            return false;
    }
}

bool operator<(const TNode& lhs, const TNode& rhs)
{
    if (!lhs.GetAttributes().Empty() || !rhs.GetAttributes().Empty()) {
        ythrow TNode::TTypeError() << "Unsupported attributes comparison";
    }

    if (!IsComparableType(lhs.GetType()) || !IsComparableType(rhs.GetType())) {
        ythrow TNode::TTypeError() << "Unsupported types for comparison: " << lhs.GetType() << " with " << rhs.GetType();
    }

    if (lhs.GetType() != rhs.GetType()) {
        return lhs.GetType() < rhs.GetType();
    }

    switch (lhs.GetType()) {
        case TNode::String:
            return lhs.AsString() < rhs.AsString();
        case TNode::Int64:
            return lhs.AsInt64() < rhs.AsInt64();
        case TNode::Uint64:
            return lhs.AsUint64() < rhs.AsUint64();
        case TNode::Double:
            return lhs.AsDouble() < rhs.AsDouble();
        case TNode::Bool:
            return lhs.AsBool() < rhs.AsBool();
        case TNode::Null:
        case TNode::Undefined:
            return false;
        default:
            Y_ABORT("Unexpected type: %d", lhs.GetType());
    }
}

bool operator>(const TNode& lhs, const TNode& rhs)
{
    return rhs < lhs;
}

bool operator<=(const TNode& lhs, const TNode& rhs)
{
    return !(lhs > rhs);
}

bool operator>=(const TNode& lhs, const TNode& rhs)
{
    return !(lhs < rhs);
}

} // namespace NNodeCmp

////////////////////////////////////////////////////////////////////////////////

TNode::TNode()
    : Value_(TUndefined{})
{ }

TNode::TNode(const char* s)
    : Value_(std::string(s))
{ }

TNode::TNode(std::string_view s)
    : Value_(std::string(s))
{ }

TNode::TNode(const std::string& s)
    : Value_(std::string(s))
{ }

TNode::TNode(std::string s)
    : Value_(std::move(s))
{ }

TNode::TNode(int i)
    : Value_(static_cast<i64>(i))
{ }


TNode::TNode(unsigned int ui)
    : Value_(static_cast<ui64>(ui))
{ }

TNode::TNode(long i)
    : Value_(static_cast<i64>(i))
{ }

TNode::TNode(unsigned long ui)
    : Value_(static_cast<ui64>(ui))
{ }

TNode::TNode(long long i)
    : Value_(static_cast<i64>(i))
{ }

TNode::TNode(unsigned long long ui)
    : Value_(static_cast<ui64>(ui))
{ }

TNode::TNode(double d)
    : Value_(d)
{ }

TNode::TNode(bool b)
    : Value_(b)
{ }

TNode::TNode(TMapType map)
    : Value_(std::move(map))
{ }

TNode::TNode(const TNode& rhs)
    : TNode()
{
    if (rhs.Attributes_) {
        CreateAttributes();
        *Attributes_ = *rhs.Attributes_;
    }
    Value_ = rhs.Value_;
}

TNode& TNode::operator=(const TNode& rhs)
{
    if (this != &rhs) {
        TNode tmp = rhs;
        Move(std::move(tmp));
    }
    return *this;
}

TNode::TNode(TNode&& rhs) noexcept
    : TNode()
{
    Move(std::move(rhs));
}

TNode& TNode::operator=(TNode&& rhs) noexcept
{
    if (this != &rhs) {
        TNode tmp = std::move(rhs);
        Move(std::move(tmp));
    }
    return *this;
}

TNode::~TNode() = default;

void TNode::Clear()
{
    ClearAttributes();
    Value_ = TUndefined();
}

bool TNode::IsString() const
{
    return std::holds_alternative<std::string>(Value_);
}

bool TNode::IsInt64() const
{
    return std::holds_alternative<i64>(Value_);
}

bool TNode::IsUint64() const
{
    return std::holds_alternative<ui64>(Value_);
}

bool TNode::IsDouble() const
{
    return std::holds_alternative<double>(Value_);
}

bool TNode::IsBool() const
{
    return std::holds_alternative<bool>(Value_);
}

bool TNode::IsList() const
{
    return std::holds_alternative<TListType>(Value_);
}

bool TNode::IsMap() const
{
    return std::holds_alternative<TMapType>(Value_);
}

bool TNode::IsEntity() const
{
    return IsNull();
}

bool TNode::IsNull() const
{
    return std::holds_alternative<TNull>(Value_);
}

bool TNode::IsUndefined() const
{
    return std::holds_alternative<TUndefined>(Value_);
}

bool TNode::HasValue() const
{
    return !IsNull() && !IsUndefined();
}

bool TNode::Empty() const
{
    switch (GetType()) {
        case String:
            return std::get<std::string>(Value_).empty();
        case List:
            return std::get<TListType>(Value_).empty();
        case Map:
            return std::get<TMapType>(Value_).empty();
        default:
            ythrow TTypeError() << "Empty() called for type " << GetType();
    }
}

size_t TNode::Size() const
{
    switch (GetType()) {
        case String:
            return std::get<std::string>(Value_).size();
        case List:
            return std::get<TListType>(Value_).size();
        case Map:
            return std::get<TMapType>(Value_).size();
        default:
            ythrow TTypeError() << "Size() called for type " << GetType();
    }
}

TNode::EType TNode::GetType() const
{
    return std::visit(TOverloaded{
        [](const TUndefined&) { return Undefined; },
        [](const std::string&) { return String; },
        [](i64) { return Int64; },
        [](ui64) { return Uint64; },
        [](double) { return Double; },
        [](bool) { return Bool; },
        [](const TListType&) { return List; },
        [](const TMapType&) { return Map; },
        [](const TNull&) { return Null; }
    }, Value_);
}

const std::string& TNode::AsString() const
{
    CheckType(String);
    return std::get<std::string>(Value_);
}

i64 TNode::AsInt64() const
{
    CheckType(Int64);
    return std::get<i64>(Value_);
}

ui64 TNode::AsUint64() const
{
    CheckType(Uint64);
    return std::get<ui64>(Value_);
}

double TNode::AsDouble() const
{
    CheckType(Double);
    return std::get<double>(Value_);
}

bool TNode::AsBool() const
{
    CheckType(Bool);
    return std::get<bool>(Value_);
}

const TNode::TListType& TNode::AsList() const
{
    CheckType(List);
    return std::get<TListType>(Value_);
}

const TNode::TMapType& TNode::AsMap() const
{
    CheckType(Map);
    return std::get<TMapType>(Value_);
}

TNode::TListType& TNode::AsList()
{
    CheckType(List);
    return std::get<TListType>(Value_);
}

TNode::TMapType& TNode::AsMap()
{
    CheckType(Map);
    return std::get<TMapType>(Value_);
}

const std::string& TNode::UncheckedAsString() const noexcept
{
    return std::get<std::string>(Value_);
}

i64 TNode::UncheckedAsInt64() const noexcept
{
    return std::get<i64>(Value_);
}

ui64 TNode::UncheckedAsUint64() const noexcept
{
    return std::get<ui64>(Value_);
}

double TNode::UncheckedAsDouble() const noexcept
{
    return std::get<double>(Value_);
}

bool TNode::UncheckedAsBool() const noexcept
{
    return std::get<bool>(Value_);
}

const TNode::TListType& TNode::UncheckedAsList() const noexcept
{
    return std::get<TListType>(Value_);
}

const TNode::TMapType& TNode::UncheckedAsMap() const noexcept
{
    return std::get<TMapType>(Value_);
}

TNode::TListType& TNode::UncheckedAsList() noexcept
{
    return std::get<TListType>(Value_);
}

TNode::TMapType& TNode::UncheckedAsMap() noexcept
{
    return std::get<TMapType>(Value_);
}

TNode TNode::CreateList()
{
    TNode node;
    node.Value_ = TListType{};
    return node;
}

TNode TNode::CreateList(TListType list)
{
    TNode node;
    node.Value_ = std::move(list);
    return node;
}

TNode TNode::CreateMap()
{
    TNode node;
    node.Value_ = TMapType{};
    return node;
}

TNode TNode::CreateMap(TMapType map)
{
    TNode node;
    node.Value_ = std::move(map);
    return node;
}

TNode TNode::CreateEntity()
{
    TNode node;
    node.Value_ = TNull{};
    return node;
}

const TNode& TNode::operator[](size_t index) const
{
    CheckType(List);
    return std::get<TListType>(Value_)[index];
}

TNode& TNode::operator[](size_t index)
{
    CheckType(List);
    return std::get<TListType>(Value_)[index];
}

const TNode& TNode::At(size_t index) const {
    CheckType(List);
    const auto& list = std::get<TListType>(Value_);
    if (index >= list.size()) {
        ythrow TLookupError() << "List out-of-range: requested index=" << index << ", but size=" << list.size();
    }
    return list[index];
}

TNode& TNode::At(size_t index) {
    CheckType(List);
    auto& list = std::get<TListType>(Value_);
    if (index >= list.size()) {
        ythrow TLookupError() << "List out-of-range: requested index=" << index << ", but size=" << list.size();
    }
    return list[index];
}

TNode& TNode::Add() &
{
    AssureList();
    return std::get<TListType>(Value_).emplace_back();
}

TNode TNode::Add() &&
{
    return std::move(Add());
}

TNode& TNode::Add(const TNode& node) &
{
    AssureList();
    std::get<TListType>(Value_).emplace_back(node);
    return *this;
}

TNode TNode::Add(const TNode& node) &&
{
    return std::move(Add(node));
}

TNode& TNode::Add(TNode&& node) &
{
    AssureList();
    std::get<TListType>(Value_).emplace_back(std::move(node));
    return *this;
}

TNode TNode::Add(TNode&& node) &&
{
    return std::move(Add(std::move(node)));
}

bool TNode::HasKey(const std::string_view key) const
{
    CheckType(Map);
    return std::get<TMapType>(Value_).contains(std::string{key});
}

TNode& TNode::operator()(const std::string& key, const TNode& value) &
{
    AssureMap();
    std::get<TMapType>(Value_)[key] = value;
    return *this;
}

TNode TNode::operator()(const std::string& key, const TNode& value) &&
{
    return std::move(operator()(key, value));
}

TNode& TNode::operator()(const std::string& key, TNode&& value) &
{
    AssureMap();
    std::get<TMapType>(Value_)[key] = std::move(value);
    return *this;
}

TNode TNode::operator()(const std::string& key, TNode&& value) &&
{
    return std::move(operator()(key, std::move(value)));
}

const TNode& TNode::operator[](const std::string_view key) const
{
    CheckType(Map);
    static TNode notFound;
    const auto& map = std::get<TMapType>(Value_);
    TMapType::const_iterator i = map.find(std::string{key});
    if (i == map.end()) {
        return notFound;
    } else {
        return i->second;
    }
}

TNode& TNode::operator[](const std::string_view key)
{
    AssureMap();
    return std::get<TMapType>(Value_)[std::string{key}];
}

const TNode& TNode::At(const std::string_view key) const {
    CheckType(Map);
    const auto& map = std::get<TMapType>(Value_);
    TMapType::const_iterator i = map.find(std::string{key});
    if (i == map.end()) {
        ythrow TLookupError() << "Cannot find key " << key;
    } else {
        return i->second;
    }
}

TNode& TNode::At(const std::string_view key) {
    CheckType(Map);
    auto& map = std::get<TMapType>(Value_);
    TMapType::iterator i = map.find(std::string{key});
    if (i == map.end()) {
        ythrow TLookupError() << "Cannot find key " << key;
    } else {
        return i->second;
    }
}

const std::string& TNode::ChildAsString(const std::string_view key) const {
    const auto& node = At(key);
    try {
        return node.AsString();
    } catch (TTypeError& e) {
        e << ", during getting key=" << key;
        throw e;
    } catch (...) {
        ythrow TTypeError() << CurrentExceptionMessage() << ", during getting key=" << key;
    }
}

i64 TNode::ChildAsInt64(const std::string_view key) const {
    const auto& node = At(key);
    try {
        return node.AsInt64();
    } catch (TTypeError& e) {
        e << ", during getting key=" << key;
        throw e;
    } catch (...) {
        ythrow TTypeError() << CurrentExceptionMessage() << ", during getting key=" << key;
    }
}

ui64 TNode::ChildAsUint64(const std::string_view key) const {
    const auto& node = At(key);
    try {
        return node.AsUint64();
    } catch (TTypeError& e) {
        e << ", during getting key=" << key;
        throw e;
    } catch (...) {
        ythrow TTypeError() << CurrentExceptionMessage() << ", during getting key=" << key;
    }
}

double TNode::ChildAsDouble(const std::string_view key) const {
    const auto& node = At(key);
    try {
        return node.AsDouble();
    } catch (TTypeError& e) {
        e << ", during getting key=" << key;
        throw e;
    } catch (...) {
        ythrow TTypeError() << CurrentExceptionMessage() << ", during getting key=" << key;
    }
}

bool TNode::ChildAsBool(const std::string_view key) const {
    const auto& node = At(key);
    try {
        return node.AsBool();
    } catch (TTypeError& e) {
        e << ", during getting key=" << key;
        throw e;
    } catch (...) {
        ythrow TTypeError() << CurrentExceptionMessage() << ", during getting key=" << key;
    }
}

const TNode::TListType& TNode::ChildAsList(const std::string_view key) const {
    const auto& node = At(key);
    try {
        return node.AsList();
    } catch (TTypeError& e) {
        e << ", during getting key=" << key;
        throw e;
    } catch (...) {
        ythrow TTypeError() << CurrentExceptionMessage() << ", during getting key=" << key;
    }
}

const TNode::TMapType& TNode::ChildAsMap(const std::string_view key) const {
    const auto& node = At(key);
    try {
        return node.AsMap();
    } catch (TTypeError& e) {
        e << ", during getting key=" << key;
        throw e;
    } catch (...) {
        ythrow TTypeError() << CurrentExceptionMessage() << ", during getting key=" << key;
    }
}

TNode::TListType& TNode::ChildAsList(const std::string_view key) {
    auto& node = At(key);
    try {
        return node.AsList();
    } catch (TTypeError& e) {
        e << ", during getting key=" << key;
        throw e;
    } catch (...) {
        ythrow TTypeError() << CurrentExceptionMessage() << ", during getting key=" << key;
    }
}

TNode::TMapType& TNode::ChildAsMap(const std::string_view key) {
    auto& node = At(key);
    try {
        return node.AsMap();
    } catch (TTypeError& e) {
        e << ", during getting key=" << key;
        throw e;
    } catch (...) {
        ythrow TTypeError() << CurrentExceptionMessage() << ", during getting key=" << key;
    }
}

const std::string& TNode::ChildAsString(size_t index) const {
    const auto& node = At(index);
    try {
        return node.AsString();
    } catch (TTypeError& e) {
        e << ", during getting index=" << index;
        throw e;
    } catch (...) {
        ythrow TTypeError() << CurrentExceptionMessage() << ", during getting index=" << index;
    }
}

i64 TNode::ChildAsInt64(size_t index) const {
    const auto& node = At(index);
    try {
        return node.AsInt64();
    } catch (TTypeError& e) {
        e << ", during getting index=" << index;
        throw e;
    } catch (...) {
        ythrow TTypeError() << CurrentExceptionMessage() << ", during getting index=" << index;
    }
}

ui64 TNode::ChildAsUint64(size_t index) const {
    const auto& node = At(index);
    try {
        return node.AsUint64();
    } catch (TTypeError& e) {
        e << ", during getting index=" << index;
        throw e;
    } catch (...) {
        ythrow TTypeError() << CurrentExceptionMessage() << ", during getting index=" << index;
    }
}

double TNode::ChildAsDouble(size_t index) const {
    const auto& node = At(index);
    try {
        return node.AsDouble();
    } catch (TTypeError& e) {
        e << ", during getting index=" << index;
        throw e;
    } catch (...) {
        ythrow TTypeError() << CurrentExceptionMessage() << ", during getting index=" << index;
    }
}

bool TNode::ChildAsBool(size_t index) const {
    const auto& node = At(index);
    try {
        return node.AsBool();
    } catch (TTypeError& e) {
        e << ", during getting index=" << index;
        throw e;
    } catch (...) {
        ythrow TTypeError() << CurrentExceptionMessage() << ", during getting index=" << index;
    }
}

const TNode::TListType& TNode::ChildAsList(size_t index) const {
    const auto& node = At(index);
    try {
        return node.AsList();
    } catch (TTypeError& e) {
        e << ", during getting index=" << index;
        throw e;
    } catch (...) {
        ythrow TTypeError() << CurrentExceptionMessage() << ", during getting index=" << index;
    }
}

const TNode::TMapType& TNode::ChildAsMap(size_t index) const {
    const auto& node = At(index);
    try {
        return node.AsMap();
    } catch (TTypeError& e) {
        e << ", during getting index=" << index;
        throw e;
    } catch (...) {
        ythrow TTypeError() << CurrentExceptionMessage() << ", during getting index=" << index;
    }
}

TNode::TListType& TNode::ChildAsList(size_t index) {
    auto& node = At(index);
    try {
        return node.AsList();
    } catch (TTypeError& e) {
        e << ", during getting index=" << index;
        throw e;
    } catch (...) {
        ythrow TTypeError() << CurrentExceptionMessage() << ", during getting index=" << index;
    }
}

TNode::TMapType& TNode::ChildAsMap(size_t index) {
    auto& node = At(index);
    try {
        return node.AsMap();
    } catch (TTypeError& e) {
        e << ", during getting index=" << index;
        throw e;
    } catch (...) {
        ythrow TTypeError() << CurrentExceptionMessage() << ", during getting index=" << index;
    }
}

bool TNode::HasAttributes() const
{
    return Attributes_ && !Attributes_->Empty();
}

void TNode::ClearAttributes()
{
    if (Attributes_) {
        Attributes_.Destroy();
    }
}

const TNode& TNode::GetAttributes() const
{
    static TNode notFound = TNode::CreateMap();
    if (!Attributes_) {
        return notFound;
    }
    return *Attributes_;
}

TNode& TNode::Attributes()
{
    if (!Attributes_) {
        CreateAttributes();
    }
    return *Attributes_;
}

void TNode::MoveWithoutAttributes(TNode&& rhs)
{
    Value_ = std::move(rhs.Value_);
    rhs.Clear();
}

void TNode::Move(TNode&& rhs)
{
    Value_ = std::move(rhs.Value_);
    Attributes_ = std::move(rhs.Attributes_);
}

void TNode::CheckType(EType type) const
{
    Y_ENSURE_EX(GetType() == type,
        TTypeError() << "TNode type " << type <<  " expected, actual type " << GetType();
    );
}

void TNode::AssureMap()
{
    if (std::holds_alternative<TUndefined>(Value_)) {
        Value_ = TMapType();
    } else {
        CheckType(Map);
    }
}

void TNode::AssureList()
{
    if (std::holds_alternative<TUndefined>(Value_)) {
        Value_ = TListType();
    } else {
        CheckType(List);
    }
}

void TNode::CreateAttributes()
{
    Attributes_ = MakeHolder<TNode>();
    Attributes_->Value_ = TMapType();
}

void TNode::Save(IOutputStream* out) const
{
    NodeToYsonStream(*this, out, NYson::EYsonFormat::Binary);
}

void TNode::Load(IInputStream* in)
{
    Clear();
    *this = NodeFromYsonStream(in, ::NYson::EYsonType::Node);
}

////////////////////////////////////////////////////////////////////////////////

bool operator==(const TNode& lhs, const TNode& rhs)
{
    if (std::holds_alternative<TNode::TUndefined>(lhs.Value_) ||
        std::holds_alternative<TNode::TUndefined>(rhs.Value_))
    {
        // TODO: should try to remove this behaviour if nobody uses it.
        return false;
    }

    if (lhs.GetType() != rhs.GetType()) {
        return false;
    }

    if (lhs.Attributes_) {
        if (rhs.Attributes_) {
            if (*lhs.Attributes_ != *rhs.Attributes_) {
                return false;
            }
        } else {
            return false;
        }
    } else {
        if (rhs.Attributes_) {
            return false;
        }
    }

    return rhs.Value_ == lhs.Value_;
}

bool operator!=(const TNode& lhs, const TNode& rhs)
{
    return !(lhs == rhs);
}

bool GetBool(const TNode& node)
{
    if (node.IsBool()) {
        return node.AsBool();
    } else if (node.IsString()) {
        return node.AsString() == "true";
    } else {
        ythrow TNode::TTypeError()
            << "GetBool(): not a boolean or string type";
    }
}

void PrintTo(const TNode& node, std::ostream* out)
{
    if (node.IsUndefined()) {
        (*out) << "NYT::TNode::Undefined";
    } else {
        (*out) << "NYT::TNode("
        << NodeToCanonicalYsonString(node)
        << ")";
    }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
