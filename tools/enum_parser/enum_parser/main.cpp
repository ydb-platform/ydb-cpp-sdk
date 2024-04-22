#include <ydb-cpp-sdk/library/json/writer/json_value.h>
#include <ydb-cpp-sdk/library/json/writer/json.h>
#include <src/library/getopt/small/last_getopt.h>

#include <tools/enum_parser/parse_enum/parse_enum.h>

#include <src/util/stream/file.h>
#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/stream/input.h>
#include <ydb-cpp-sdk/util/stream/mem.h>

#include <src/util/charset/wide.h>
#include <ydb-cpp-sdk/util/string/builder.h>
#include <ydb-cpp-sdk/util/string/escape.h>
#include <ydb-cpp-sdk/util/string/strip.h>
#include <ydb-cpp-sdk/util/string/cast.h>
#include <src/util/string/join.h>
#include <ydb-cpp-sdk/util/string/subst.h>
#include <string>

#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>
#include <src/util/system/fs.h>
#include <src/util/folder/path.h>

#include <fstream>

void WriteHeader(const std::string& headerName, std::ostream& out, std::ostream* headerOutPtr = nullptr) {
    out << "// This file was auto-generated. Do not edit!!!\n";
    out << "#include " << headerName << "\n";
    out << "#include <tools/enum_parser/enum_serialization_runtime/enum_runtime.h>\n\n";
    out << "#include <tools/enum_parser/enum_parser/stdlib_deps.h>\n\n";
    out << "#include <ydb-cpp-sdk/util/generic/typetraits.h>\n";
    out << "#include <ydb-cpp-sdk/util/generic/singleton.h>\n";
    out << "#include <string>\n";
    out << "#include <vector>\n";
    out << "#include <map>\n";
    out << "#include <span>\n";
    out << "#include <ydb-cpp-sdk/util/string/cast.h>\n";
    out << "#include <ydb-cpp-sdk/util/stream/output.h>\n\n";

    if (headerOutPtr) {
        auto& outHeader = *headerOutPtr;
        outHeader << "// This file was auto-generated. Do not edit!!!\n";
        outHeader << "#pragma once\n\n";
        outHeader << "#include " << headerName << "\n";
    }
}

static inline void JsonEscape(std::string& s) {
    SubstGlobal(s, "\\", "\\\\");
    SubstGlobal(s, "\"", "\\\"");
    SubstGlobal(s, "\r", "\\r");
    SubstGlobal(s, "\n", "\\n");
    SubstGlobal(s, "\t", "\\t");
}

static inline std::string JsonQuote(const std::string& s) {
    std::string quoted = s;
    JsonEscape(quoted);
    return "\"" + quoted + "\""; // do not use .Quote() here, it performs escaping!
}


/// Simplifed JSON map encoder for generic types
template<typename T>
void OutKey(IOutputStream& out, const std::string& key, const T& value, bool escape = true) {
    std::string quoted = ToString(value);
    if (escape) {
        quoted = JsonQuote(quoted);
    }
    out << "\"" << key << "\": " << quoted << ",\n";
}

/// Simplifed JSON map encoder for optional
void OutKey(IOutputStream& out, const std::string& key, const std::optional<std::string>& value) {
    std::string quoted;
    if (value.has_value()) {
        quoted = JsonQuote(ToString(*value));
    } else {
        quoted = "null";
    }
    out << "\"" << key << "\": " << quoted << ",\n";
}


/// Simplifed JSON map encoder for bool values
void OutKey(IOutputStream& out, const std::string& key, const bool& value) {
    out << "\"" << key << "\": " << (value ? "true" : "false") << ",\n";
}


/// Simplifed JSON map encoder for array items
template<typename T>
void OutItem(IOutputStream& out, const T& value, bool escape = true) {
    std::string quoted = ToString(value);
    if (escape) {
        quoted = JsonQuote(quoted);
    }
    out << quoted << ",\n";
}

/// Cut trailing ",\n" or ","
static inline void FinishItems(TStringStream& out) {
    std::string& s = out.Str();
    if (s.ends_with(",\n")) {
        s.erase(s.size() - 2, 2);
    }
    if (s.ends_with(",")) {
        s.pop_back();
    }
}


static inline void OpenMap(TStringStream& out) {
    out << "{\n";
}

static inline void CloseMap(TStringStream& out) {
    out << "}\n";
}

static inline void OpenArray(TStringStream& out) {
    out << "[\n";
}

static inline void CloseArray(TStringStream& out) {
    out << "]\n";
}

static std::string WrapStringBuf(const std::string_view str) {
    return TStringBuilder() << "\"" << str << "\"sv";
}

void GenerateEnum(
    const TEnumParser::TEnum& en,
    std::ostream& out,
    IOutputStream* jsonEnumOut = nullptr,
    std::ostream* headerOutPtr = nullptr
) {
    TStringStream jEnum;
    OpenMap(jEnum);

    size_t count = en.Items.size();
    OutKey(jEnum, "count", count);
    const std::string name = TEnumParser::ScopeStr(en.Scope) + en.CppName;
    OutKey(jEnum, "full_name", name);
    OutKey(jEnum, "cpp_name", en.CppName);
    TStringStream scopeJson;
    OpenArray(scopeJson);
    for (const auto& scopeItem : en.Scope) {
        OutItem(scopeJson, scopeItem);
    }
    FinishItems(scopeJson);
    CloseArray(scopeJson);

    OutKey(jEnum, "scope", scopeJson.Str(), false);
    OutKey(jEnum, "enum_class", en.EnumClass);

    TEnumParser::TScope outerScope = en.Scope;
    if (en.EnumClass) {
        outerScope.push_back(en.CppName);
    }

    std::string outerScopeStr = TEnumParser::ScopeStr(outerScope);

    std::string cName = name;
    SubstGlobal(cName, "::", "");

    out << "// I/O for " << name << "\n";

    std::string nsName = "N" + cName + "Private";

    out << "namespace {\n" "using namespace std::literals;\n" "namespace " << nsName << " {\n";

    std::vector<std::string> nameInitializerPairs;
    std::vector<std::pair<std::string, std::string>> valueInitializerPairsUnsorted;  // data, sort_key
    std::vector<std::string> cppNamesInitializer;

    TStringStream jItems;
    OpenArray(jItems);

    for (const auto& it : en.Items) {
        TStringStream jEnumItem;
        OpenMap(jEnumItem);

        OutKey(jEnumItem, "cpp_name", it.CppName);
        OutKey(jEnumItem, "value", it.Value);
        OutKey(jEnumItem, "comment_text", it.CommentText);

        TStringStream jAliases;
        OpenArray(jAliases);

        std::string strValue = it.CppName;
        if (!it.Aliases.empty()) {
            // first alias is main
            strValue = it.Aliases[0];
            OutKey(jEnumItem, "str_value", strValue);
        }
        nameInitializerPairs.push_back("TNameBufsBase::EnumStringPair(" + outerScopeStr + it.CppName + ", " + WrapStringBuf(strValue) + ")");
        cppNamesInitializer.push_back(WrapStringBuf(it.CppName));

        for (const auto& alias : it.Aliases) {
            valueInitializerPairsUnsorted.emplace_back("TNameBufsBase::EnumStringPair(" + outerScopeStr + it.CppName + ", " + WrapStringBuf(alias) + ")", UnescapeC(alias));
            OutItem(jAliases, alias);
        }
        FinishItems(jAliases);
        CloseArray(jAliases);

        if (it.Aliases.empty()) {
            valueInitializerPairsUnsorted.emplace_back("TNameBufsBase::EnumStringPair(" + outerScopeStr + it.CppName + ", " + WrapStringBuf(it.CppName) + ")", it.CppName);
        }
        OutKey(jEnumItem, "aliases", jAliases.Str(), false);

        FinishItems(jEnumItem);
        CloseMap(jEnumItem);

        OutItem(jItems, jEnumItem.Str(), false);
    }
    FinishItems(jItems);
    CloseArray(jItems);
    OutKey(jEnum, "items", jItems.Str(), false);

    const std::string nsNameBufsClass = nsName + "::TNameBufs";

    auto defineConstArray = [&out, payloadCache = std::map<std::pair<std::string, std::vector<std::string>>, std::string>()](const std::string_view indent, const std::string_view elementType, const std::string_view name, const std::vector<std::string>& items) mutable {
        if (items.empty()) { // ISO C++ forbids zero-size array
            out << indent << "static constexpr const std::span<const " << elementType << "> " << name << ";\n";
        } else {
            // try to reuse one of the previous payload arrays
            const auto inserted = payloadCache.emplace(std::make_pair(elementType, items), ToString(name) + "_PAYLOAD");
            const std::string& payloadStorageName = inserted.first->second;
            if (inserted.second) { // new array content or type
                out << indent << "static constexpr const " << elementType << " " << payloadStorageName << "[" << items.size() << "]{\n";
                for (const auto& it : items) {
                    out << indent << "    " << it << ",\n";
                }
                out << indent << "};\n";
            }
            out << indent << "static constexpr const std::span<const " << elementType << "> " << name << "{" << payloadStorageName << "};\n";
        }
        out << "\n";
    };

    out << "    using TNameBufsBase = ::NEnumSerializationRuntime::TEnumDescription<" << name << ">;\n\n";

    // Initialization data
    {
        out << "    static constexpr const std::array<TNameBufsBase::TEnumStringPair, " << nameInitializerPairs.size() << "> NAMES_INITIALIZATION_PAIRS_PAYLOAD = ::NEnumSerializationRuntime::TryStableSortKeys(";
        out << "std::array<TNameBufsBase::TEnumStringPair, " << nameInitializerPairs.size() << ">{{\n";
        for (const auto& it : nameInitializerPairs) {
            out << "        " << it << ",\n";
        }
        out << "    }});\n";
        out << "    " << "static constexpr const std::span<const TNameBufsBase::TEnumStringPair> " << "NAMES_INITIALIZATION_PAIRS{NAMES_INITIALIZATION_PAIRS_PAYLOAD};\n\n";
    }
    {
        StableSortBy(valueInitializerPairsUnsorted, [](const auto& pair) -> const std::string& { return pair.second; });
        std::vector<std::string> valueInitializerPairs;
        valueInitializerPairs.reserve(valueInitializerPairsUnsorted.size());
        for (auto& [value, _] : valueInitializerPairsUnsorted) {
            valueInitializerPairs.push_back(std::move(value));
        }
        defineConstArray("    ", "TNameBufsBase::TEnumStringPair", "VALUES_INITIALIZATION_PAIRS", valueInitializerPairs);
    }
    defineConstArray("    ", "std::string_view", "CPP_NAMES_INITIALIZATION_ARRAY", cppNamesInitializer);

    out << "    static constexpr const TNameBufsBase::TInitializationData ENUM_INITIALIZATION_DATA{\n";
    out << "        NAMES_INITIALIZATION_PAIRS,\n";
    out << "        VALUES_INITIALIZATION_PAIRS,\n";
    out << "        CPP_NAMES_INITIALIZATION_ARRAY,\n";
    out << "        " << WrapStringBuf(outerScopeStr) << ",\n";
    out << "        " << WrapStringBuf(name) << "\n";
    out << "    };\n\n";

    // Properties
    out << "    static constexpr ::NEnumSerializationRuntime::ESortOrder NAMES_ORDER = ::NEnumSerializationRuntime::GetKeyFieldSortOrder(NAMES_INITIALIZATION_PAIRS);\n";
    out << "    static constexpr ::NEnumSerializationRuntime::ESortOrder VALUES_ORDER = ::NEnumSerializationRuntime::GetNameFieldSortOrder(VALUES_INITIALIZATION_PAIRS);\n";
    out << "\n";

    // Data holder
    out << "    class TNameBufs : public ::NEnumSerializationRuntime::TEnumDescription<" << name << "> {\n";
    out << "    public:\n";
    out << "        using TBase = ::NEnumSerializationRuntime::TEnumDescription<" << name << ">;\n\n";
    out << "        inline TNameBufs();\n\n";

    // Reference initialization data from class
    out << "        static constexpr const TNameBufsBase::TInitializationData& EnumInitializationData = ENUM_INITIALIZATION_DATA;\n";
    out << "        static constexpr ::NEnumSerializationRuntime::ESortOrder NamesOrder = NAMES_ORDER;\n";
    out << "        static constexpr ::NEnumSerializationRuntime::ESortOrder ValuesOrder = VALUES_ORDER;\n\n";

    // Instance
    out << "        static inline const TNameBufs& Instance() {\n";
    out << "            return *SingletonWithPriority<TNameBufs, 0>();\n"; // destroy enum serializers last, because it may be used from destructor of another global object
    out << "        }\n";
    out << "    };\n\n";

    // Constructor
    out << "    inline TNameBufs::TNameBufs()\n";
    out << "        : TBase(TNameBufs::EnumInitializationData)\n";
    out << "    {\n";
    out << "    }\n\n";

    out << "}}\n\n";

    if (headerOutPtr) {
        (*headerOutPtr) << "// I/O for " << name << "\n";
    }

    // outer ToString
    if (headerOutPtr) {
        (*headerOutPtr) << "const std::string& ToString(" << name << ");\n";
        (*headerOutPtr) << "Y_FORCE_INLINE std::string_view ToStringBuf(" << name << " e) {\n";
        (*headerOutPtr) << "    return ::NEnumSerializationRuntime::ToStringBuf<" << name << ">(e);\n";
        (*headerOutPtr) << "}\n";
    }
    out << "const std::string& ToString(" << name << " x) {\n";
    out << "    const " << nsNameBufsClass << "& names = " << nsNameBufsClass << "::Instance();\n";
    out << "    return names.ToString(x);\n";
    out << "}\n\n";

    // specialization for internal FromStringImpl
    out << "template<>\n";
    out << name << " FromStringImpl<" << name << ">(const char* data, size_t len) {\n";
    out << "    return ::NEnumSerializationRuntime::DispatchFromStringImplFn<" << nsNameBufsClass << ", " << name << ">(data, len);\n";
    out << "}\n\n";

    // specialization for internal TryFromStringImpl
    out << "template<>\n";
    out << "bool TryFromStringImpl<" << name << ">(const char* data, size_t len, " << name << "& result) {\n";
    out << "    return ::NEnumSerializationRuntime::DispatchTryFromStringImplFn<" << nsNameBufsClass << ", " << name << ">(data, len, result);\n";
    out << "}\n\n";

    // outer FromString
    if (headerOutPtr) {
        (*headerOutPtr) << "bool FromString(const std::string& name, " << name << "& ret);\n";
    }
    out << "bool FromString(const std::string& name, " << name << "& ret) {\n";
    out << "    return ::TryFromStringImpl<" << name << ">(name.data(), name.size(), ret);\n";
    out << "}\n\n";

    // outer FromString
    if (headerOutPtr) {
        (*headerOutPtr) << "bool FromString(const std::string_view& name, " << name << "& ret);\n";
    }
    out << "bool FromString(const std::string_view& name, " << name << "& ret) {\n";
    out << "    return ::TryFromStringImpl<" << name << ">(name.data(), name.size(), ret);\n";
    out << "}\n\n";

    // outer Out
    out << "template<>\n";
    out << "void Out<" << name << ">(IOutputStream& os, TTypeTraits<" << name << ">::TFuncParam n) {\n";
    out << "    return ::NEnumSerializationRuntime::DispatchOutFn<" << nsNameBufsClass << ">(os, n);\n";
    out << "}\n\n";

    // specializations for NEnumSerializationRuntime function family
    out << "namespace NEnumSerializationRuntime {\n";
    // template<> ToStringBuf
    out << "    template<>\n";
    out << "    std::string_view ToStringBuf<" << name << ">(" << name << " e) {\n";
    out << "        return ::NEnumSerializationRuntime::DispatchToStringBufFn<" << nsNameBufsClass << ">(e);\n";
    out << "    }\n\n";

    // template<> GetEnumAllValues
    out << "    template<>\n";
    out << "    TMappedArrayView<" << name <<"> GetEnumAllValuesImpl<" << name << ">() {\n";
    out << "        const " << nsNameBufsClass << "& names = " << nsNameBufsClass << "::Instance();\n";
    out << "        return names.AllEnumValues();\n";
    out << "    }\n\n";

    // template<> GetEnumAllNames
    out << "    template<>\n";
    out << "    const std::string& GetEnumAllNamesImpl<" << name << ">() {\n";
    out << "        const " << nsNameBufsClass << "& names = " << nsNameBufsClass << "::Instance();\n";
    out << "        return names.AllEnumNames();\n";
    out << "    }\n\n";

    // template<> GetEnumNames<EnumType>
    out << "    template<>\n";
    out << "    TMappedDictView<" << name << ", std::string> GetEnumNamesImpl<" << name << ">() {\n";
    out << "        const " << nsNameBufsClass << "& names = " << nsNameBufsClass << "::Instance();\n";
    out << "        return names.EnumNames();\n";
    out << "    }\n\n";

    // template<> GetEnumAllCppNames, see IGNIETFERRO-534
    out << "    template<>\n";
    out << "    const std::vector<std::string>& GetEnumAllCppNamesImpl<" << name << ">() {\n";
    out << "        const " << nsNameBufsClass << "& names = " << nsNameBufsClass << "::Instance();\n";
    out << "        return names.AllEnumCppNames();\n";
    out << "    }\n";

    out << "}\n\n";

    if (headerOutPtr) {
        // <EnumType>Count
        auto& outHeader = *headerOutPtr;
        outHeader << "template <>\n";
        outHeader << "constexpr size_t GetEnumItemsCount<" << name << ">() {\n";
        outHeader << "    return " << en.Items.size() << ";\n";
        outHeader << "}\n";
    }

    FinishItems(jEnum);
    jEnum << "}\n";

    if (jsonEnumOut) {
        *jsonEnumOut << jEnum.Str();
    }
}

int main(int argc, char** argv) {
    try {
        using namespace NLastGetopt;
        TOpts opts = NLastGetopt::TOpts::Default();
        opts.AddHelpOption();

        std::string outputFileName;
        std::string outputHeaderFileName;
        std::string outputJsonFileName;
        std::string includePath;
        opts.AddLongOption('o', "output").OptionalArgument("<output-file>").StoreResult(&outputFileName)
            .Help(
                "Output generated code to specified file.\n"
                "When not set, standard output is used."
            );
        opts.AddLongOption('h', "header").OptionalArgument("<output-header>").StoreResult(&outputHeaderFileName)
            .Help(
                "Generate appropriate header to specified file.\n"
                "Works only if output file specified."
            );
        opts.AddLongOption("include-path").OptionalArgument("<header-path>").StoreResult(&includePath)
            .Help(
                "Include input header using this path in angle brackets.\n"
                "When not set, header basename is used in double quotes."
            );

        opts.AddLongOption('j', "json-output").OptionalArgument("<json-output>").StoreResult(&outputJsonFileName)
            .Help(
                "Generate enum data in JSON format."
            );

        opts.SetFreeArgsNum(1);
        opts.SetFreeArgTitle(0, "<input-file>", "Input header file with enum declarations");

        TOptsParseResult res(&opts, argc, argv);

        std::vector<std::string> freeArgs = res.GetFreeArgs();
        std::string inputFileName = freeArgs[0];

        THolder<std::ostream> hOut;
        std::ostream* out = &std::cout;

        THolder<std::ostream> headerOut;

        THolder<std::ostream> jsonOut;

        if (!outputFileName.empty()) {
            NFs::Remove(outputFileName.c_str());
            hOut.Reset(new std::ofstream(outputFileName));
            out = hOut.Get();

            if (!outputHeaderFileName.empty()) {
                headerOut.Reset(new std::ofstream(outputHeaderFileName));
            }

            if (!outputJsonFileName.empty()) {
                jsonOut.Reset(new std::ofstream(outputJsonFileName));
            }
        }

        if (includePath.empty()) {
            includePath = std::string() + '"' + TFsPath(inputFileName).Basename() + '"';
        } else {
            includePath = std::string() + '<' + includePath + '>';
        }

        TEnumParser parser(inputFileName);
        WriteHeader(includePath, *out, headerOut.Get());

        TStringStream jEnums;
        OpenArray(jEnums);

        for (const auto& en : parser.Enums) {
            if (en.CppName.empty()) {
                // skip unnamed enum declarations
                continue;
            }

            TStringStream jEnum;
            GenerateEnum(en, *out, &jEnum, headerOut.Get());
            OutItem(jEnums, jEnum.Str(), false);
        }
        FinishItems(jEnums);
        CloseArray(jEnums);

        if (jsonOut) {
            *jsonOut << jEnums.Str() << std::endl;
        }

        return 0;
    } catch (...) {
        std::cerr << CurrentExceptionMessage() << std::endl;
    }

    return 1;
}
