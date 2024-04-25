#include "enum_runtime.h"

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/string/builder.h>

#include <ydb-cpp-sdk/util/generic/algorithm.h>
=======
#include <src/util/string/builder.h>

#include <src/util/generic/algorithm.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/string/builder.h>

#include <ydb-cpp-sdk/util/generic/algorithm.h>
>>>>>>> origin/main

namespace NEnumSerializationRuntime {
    template <typename TEnumRepresentationType>
    [[noreturn]] static void ThrowUndefinedValueException(const TEnumRepresentationType key, const std::string_view className) {
        throw yexception() << "Undefined value " << key << " in " << className << ". ";
    }

    template <typename TEnumRepresentationType>
    const std::string& TEnumDescriptionBase<TEnumRepresentationType>::ToString(TRepresentationType key) const {
        const auto it = Names.find(key);
        if (Y_LIKELY(it != Names.end())) {
            return it->second;
        }
        ThrowUndefinedValueException(key, ClassName);
    }

    template <typename TEnumRepresentationType>
    std::pair<bool, TEnumRepresentationType> TEnumDescriptionBase<TEnumRepresentationType>::TryFromString(const std::string_view name) const {
        const auto it = Values.find(name);
        if (it != Values.end()) {
            return {true, it->second};
        }
        return {false, TRepresentationType()};
    }

    template <class TContainer, class TNeedle, class TGetKey>
    static typename TContainer::value_type const* FindPtrInSortedContainer(const TContainer& vec, const TNeedle& needle, TGetKey&& getKey) {
        const auto it = LowerBoundBy(vec.begin(), vec.end(), needle, getKey);
        if (it == vec.end()) {
            return nullptr;
        }
        if (getKey(*it) != needle) {
            return nullptr;
        }
        return std::addressof(*it);
    }

    template <typename TEnumRepresentationType>
    std::pair<bool, TEnumRepresentationType> TEnumDescriptionBase<TEnumRepresentationType>::TryFromStringSorted(const std::string_view name, const TInitializationData& enumInitData) {
        const auto& vec = enumInitData.ValuesInitializer;
        const auto* ptr = FindPtrInSortedContainer(vec, name, std::mem_fn(&TEnumStringPair::Name));
        if (ptr) {
            return {true, ptr->Key};
        }
        return {false, TRepresentationType()};
    }

    template <typename TEnumRepresentationType>
    std::pair<bool, TEnumRepresentationType> TEnumDescriptionBase<TEnumRepresentationType>::TryFromStringFullScan(const std::string_view name, const TInitializationData& enumInitData) {
        const auto& vec = enumInitData.ValuesInitializer;
        const auto* ptr = FindIfPtr(vec, [&](const auto& item) { return item.Name == name; });
        if (ptr) {
            return {true, ptr->Key};
        }
        return {false, TRepresentationType()};
    }

    [[noreturn]] static void ThrowUndefinedNameException(const std::string_view name, const std::string_view className, const std::string_view allEnumNames) {
        ythrow yexception() << "Key '" << name << "' not found in enum " << className << ". Valid options are: " << allEnumNames << ". ";
    }

    template <typename TEnumRepresentationType>
    [[noreturn]] static void ThrowUndefinedNameException(const std::string_view name, const typename TEnumDescriptionBase<TEnumRepresentationType>::TInitializationData& enumInitData) {
        auto exc = __LOCATION__ + yexception() << "Key '" << name << "' not found in enum " << enumInitData.ClassName << ". Valid options are: ";
        const auto& vec = enumInitData.NamesInitializer;
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i != 0) {
                exc << ", ";
            }
            exc << '\'' << vec[i].Name << '\'';
        }
        exc << ". ";
        throw exc;
    }

    template <typename TEnumRepresentationType>
    auto TEnumDescriptionBase<TEnumRepresentationType>::FromString(const std::string_view name) const -> TRepresentationType {
        const auto findResult = TryFromString(name);
        if (Y_LIKELY(findResult.first)) {
            return findResult.second;
        }
        ThrowUndefinedNameException(name, ClassName, AllEnumNames());
    }

    template <typename TEnumRepresentationType>
    TEnumRepresentationType TEnumDescriptionBase<TEnumRepresentationType>::FromStringFullScan(const std::string_view name, const TInitializationData& enumInitData) {
        const auto findResult = TryFromStringFullScan(name, enumInitData);
        if (Y_LIKELY(findResult.first)) {
            return findResult.second;
        }
        ThrowUndefinedNameException<TEnumRepresentationType>(name, enumInitData);
    }

    template <typename TEnumRepresentationType>
    TEnumRepresentationType TEnumDescriptionBase<TEnumRepresentationType>::FromStringSorted(const std::string_view name, const TInitializationData& enumInitData) {
        const auto findResult = TryFromStringSorted(name, enumInitData);
        if (Y_LIKELY(findResult.first)) {
            return findResult.second;
        }
        ThrowUndefinedNameException<TEnumRepresentationType>(name, enumInitData);
    }

    template <typename TEnumRepresentationType>
    std::string_view TEnumDescriptionBase<TEnumRepresentationType>::ToStringBuf(TRepresentationType key) const {
        return this->ToString(key);
    }

    template <typename TEnumRepresentationType>
    std::string_view TEnumDescriptionBase<TEnumRepresentationType>::ToStringBufFullScan(const TRepresentationType key, const TInitializationData& enumInitData) {
        const auto& vec = enumInitData.NamesInitializer;
        const auto* ptr = FindIfPtr(vec, [&](const auto& item) { return item.Key == key; });
        if (Y_UNLIKELY(!ptr)) {
            ThrowUndefinedValueException(key, enumInitData.ClassName);
        }
        return ptr->Name;
    }

    template <typename TEnumRepresentationType>
    std::string_view TEnumDescriptionBase<TEnumRepresentationType>::ToStringBufSorted(const TRepresentationType key, const TInitializationData& enumInitData) {
        const auto& vec = enumInitData.NamesInitializer;
        const auto* ptr = FindPtrInSortedContainer(vec, key, std::mem_fn(&TEnumStringPair::Key));
        if (Y_UNLIKELY(!ptr)) {
            ThrowUndefinedValueException(key, enumInitData.ClassName);
        }
        return ptr->Name;
    }

    template <typename TEnumRepresentationType>
    std::string_view TEnumDescriptionBase<TEnumRepresentationType>::ToStringBufDirect(const TRepresentationType key, const TInitializationData& enumInitData) {
        const auto& vec = enumInitData.NamesInitializer;
        if (Y_UNLIKELY(vec.empty() || key < vec.front().Key)) {
            ThrowUndefinedValueException(key, enumInitData.ClassName);
        }
        const size_t index = static_cast<size_t>(key - vec.front().Key);
        if (Y_UNLIKELY(index >= vec.size())) {
            ThrowUndefinedValueException(key, enumInitData.ClassName);
        }
        return vec[index].Name;
    }

    template <typename TEnumRepresentationType>
    void TEnumDescriptionBase<TEnumRepresentationType>::Out(IOutputStream* os, const TRepresentationType key) const {
        (*os) << this->ToStringBuf(key);
    }

    template <typename TEnumRepresentationType>
    void TEnumDescriptionBase<TEnumRepresentationType>::OutFullScan(IOutputStream* os, const TRepresentationType key, const TInitializationData& enumInitData) {
        (*os) << ToStringBufFullScan(key, enumInitData);
    }

    template <typename TEnumRepresentationType>
    void TEnumDescriptionBase<TEnumRepresentationType>::OutSorted(IOutputStream* os, const TRepresentationType key, const TInitializationData& enumInitData) {
        (*os) << ToStringBufSorted(key, enumInitData);
    }

    template <typename TEnumRepresentationType>
    void TEnumDescriptionBase<TEnumRepresentationType>::OutDirect(IOutputStream* os, const TRepresentationType key, const TInitializationData& enumInitData) {
        (*os) << ToStringBufDirect(key, enumInitData);
    }

    template <typename TEnumRepresentationType>
    TEnumDescriptionBase<TEnumRepresentationType>::TEnumDescriptionBase(const TInitializationData& enumInitData)
        : ClassName(enumInitData.ClassName)
    {
        const std::span<const TEnumStringPair>& namesInitializer = enumInitData.NamesInitializer;
        const std::span<const TEnumStringPair>& valuesInitializer = enumInitData.ValuesInitializer;
        const std::span<const std::string_view>& cppNamesInitializer = enumInitData.CppNamesInitializer;

        std::map<TRepresentationType, std::string> mapValueToName;
        std::map<std::string_view, TRepresentationType> mapNameToValue;
        for (const TEnumStringPair& it : namesInitializer) {
            mapValueToName.emplace(it.Key, std::string(it.Name));
        }
        for (const TEnumStringPair& it : valuesInitializer) {
            mapNameToValue.emplace(it.Name, it.Key);
        }
        Names = std::move(mapValueToName);
        Values = std::move(mapNameToValue);

        AllValues.reserve(Names.size());
        for (const auto& it : Names) {
            if (!AllNames.empty()) {
                AllNames += ", ";
            }
            AllNames += TStringBuilder() << '\'' << it.second << '\'';
            AllValues.push_back(it.first);
        }

        AllCppNames.reserve(cppNamesInitializer.size());
        for (const auto& cn : cppNamesInitializer) {
            AllCppNames.push_back(TStringBuilder() << enumInitData.CppNamesPrefix << cn);
        }
    }

    template <typename TEnumRepresentationType>
    TEnumDescriptionBase<TEnumRepresentationType>::~TEnumDescriptionBase() = default;

    // explicit instantiation
    template class TEnumDescriptionBase<int>;
    template class TEnumDescriptionBase<unsigned>;
    template class TEnumDescriptionBase<long long>;
    template class TEnumDescriptionBase<unsigned long long>;
}
