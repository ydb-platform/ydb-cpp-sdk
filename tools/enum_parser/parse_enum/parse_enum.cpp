#include "parse_enum.h"

#include <library/cpp/cppparser/parser.h>

#include <util/generic/yexception.h>
#include <util/stream/file.h>
#include <util/string/escape.h>

/**
 * Parse C-style strings inside multiline comments
 **/
class TValuesContext: public TCppFullSax {
public:
    void DoString(const TText& text) override {
        Values.push_back(text.Data);
    }

    ~TValuesContext() override {
    }

    std::vector<std::string> Values;
};

static std::vector<std::string> ParseEnumValues(const std::string& strValues) {
    std::vector<std::string> result;

    TValuesContext ctx;
    TCppSaxParser parser(&ctx);
    TMemoryInput in(strValues.data(), strValues.size());
    TransferData(static_cast<IInputStream*>(&in), &parser);
    parser.Finish();
    for (const auto& value : ctx.Values) {
        Y_ENSURE(value.size() >= 2, "Invalid C-style string. ");
        std::string dequoted = value.substr(1, value.size() - 2);
        // TODO: support C-unescaping
        result.push_back(dequoted);
    }
    return result;
}

/**
 * Parse C++ fragment with one enum
 **/
class TEnumContext: public TCppFullSax {
public:
    typedef TEnumParser::TItem TItem;
    typedef TEnumParser::TEnum TEnum;

    TEnumContext(TEnum& currentEnum)
        : CurrentEnum(currentEnum)
    {
    }

    ~TEnumContext() override {
    }

    void AddEnumItem() {
        if (CurrentItem.CppName.empty()) {
            // uninitialized element should have no value too
            Y_ASSERT(!CurrentItem.Value.has_value());
            return;
        }

        // enum item C++ name should not be empty
        Y_ASSERT(!CurrentItem.CppName.empty());
        CurrentItem.NormalizeValue();
        CurrentEnum.Items.push_back(CurrentItem);
        CurrentItem.Clear();
        InEnumState = Begin;
    }

    template<class T>
    void AppendValue(const T& text) {
        // by pg@ advice, do not parse enum value
        // leave it to C++ compiler to parse/interpret

        if (!CurrentItem.Value)
            CurrentItem.Value = std::string();

        *CurrentItem.Value += text;
    }

    void DoEnd() override {
        AddEnumItem();
    }

    void DoWhiteSpace(const TText& text) override {
        if (InValue == InEnumState || InValueCall == InEnumState) {
            AppendValue(text.Data);
        }
    }

    void DoSyntax(const TText& text) override {
        // For some reason, parser sometimes passes chunks like '{};' here,
        // so we handle each symbol separately.
        for (const char& sym : text.Data) {
            if ('{' == sym && InValue != InEnumState && InValueCall != InEnumState) {
                BodyDetected = true;
                continue;
            } else if ('=' == sym && InValueCall != InEnumState) {
                InEnumState = InValue;
                continue;
            } else if (('(' == sym || '{' == sym) && (InValue == InEnumState || InValueCall == InEnumState)) {
                // there may be constexpr function / constructor / macro call in value part,
                // handle them appropriately
                InEnumState = InValueCall;
                ++BracesBalance;
                AppendValue(sym);
                continue;
            } else if ((')' == sym || '}' == sym) && InValueCall == InEnumState) {
                if (!--BracesBalance) {
                    InEnumState = InValue;
                }
                AppendValue(sym);
                continue;
            } else if ((',' == sym || '}' == sym) && InValueCall != InEnumState) {
                AddEnumItem();
                continue;
            } else if (InValue == InEnumState || InValueCall == InEnumState) {
                AppendValue(sym);
            }
        }
    }

    void DoName(const TText& text) override {
        if (!BodyDetected) {
            return;
        }

        if (InValue == InEnumState || InValueCall == InEnumState) {
            AppendValue(text.Data);
            return;
        }

        CurrentItem.CppName = text.Data;
        InEnumState = AfterCppName;
    }

    void DoMultiLineComment(const TText& text) override {
        Y_ENSURE(text.Data.size() >= 4, "Invalid multiline comment " << NUtils::Quote(text.Data) << ". ");
        std::string commentText = text.Data.substr(2, text.Data.size() - 4);
        commentText = StripString(commentText);
        CurrentItem.CommentText = commentText;
        CurrentItem.Aliases = ParseEnumValues(commentText);

        if (!CurrentItem.Aliases.empty() && CurrentItem.CppName.empty()) {
            // this means we process multiline comment when item name was not set yet.
            ythrow yexception() << "Are you hit with https://clubs.at.yandex-team.ru/stackoverflow/2603 typo? ";
        }
    }

    bool BodyDetected = false;
    enum EInEnumState {
        Begin,
        AfterCppName,
        InValue,
        InValueCall,
        End,
    };
    EInEnumState InEnumState = Begin;

    TEnum& CurrentEnum;
    TItem CurrentItem;

    size_t BracesBalance = 0;
};

/**
 * Parse C++ file
 **/
class TCppContext: public TCppFullSax {
public:
    typedef TEnumParser::TScope TScope;
    typedef TEnumParser::TItem TItem;
    typedef TEnumParser::TEnum TEnum;
    typedef TEnumParser::TEnums TEnums;

    const std::string NAMESPACE = "<namespace>";
    const std::string CLASS = "<class>";
    const std::string STRUCT = "<struct>";
    const std::string ENUM = "<enum>";
    const std::string BLOCK = "<block>";

    TCppContext(const char* data, const std::string& sourceFileName = std::string())
        : Data(data)
        , SourceFileName(sourceFileName)
    {
    }

    ~TCppContext() override {
    }

    void DoSyntax(const TText& text) override {
        // For some reason, parser sometimes passes chunks like '{};' here,
        // so we handle each symbol separately.
        const std::string& syn = text.Data;
        if (syn == "::" && InCompositeNamespace) {
            LastScope += syn;
            InCompositeNamespace = false;
            ScopeDeclaration = true;
            return;
        }
        for (size_t i = 0; i < syn.size(); ++i) {
            if ('{' == syn[i]) {
                OnEnterScope(text.Offset + i);
                if (InEnum) {
                    CurrentEnum.BodyDetected = true;
                }
            } else if ('}' == syn[i]) {
                OnLeaveScope(text.Offset + i);
            } else if (';' == syn[i]) {
                // Handle SEARCH-1392
                if (InEnum && !CurrentEnum.BodyDetected) {
                    CurrentEnum.ForwardDeclaration = true;
                    InEnum = false;
                }
            }
        }
    }

    void DoKeyword(const TText& text) override {
        if (text.Data == "enum") {
            Y_ENSURE(!InEnum, "Enums cannot be nested. ");
            InEnum = true;
            EnumPos = text.Offset;
            CurrentEnum.Clear();
            CurrentEnum.Scope = Scope;
            ScopeDeclaration = true;
            NextScopeName = ENUM;
            //PrintScope();
        } else if (text.Data == "class") {
            if (InEnum) {
                CurrentEnum.EnumClass = true;
                return;
            }
            NextScopeName = CLASS;
            ScopeDeclaration = true;
            //PrintScope();
        } else if (text.Data == "struct") {
            if (InEnum) {
                CurrentEnum.EnumClass = true;
                return;
            }
            NextScopeName = STRUCT;
            ScopeDeclaration = true;
            //PrintScope();
        } else if (text.Data == "namespace") {
            NextScopeName = NAMESPACE;
            LastScope.clear();
            ScopeDeclaration = true;
            //PrintScope();
        }
    }

    void DoName(const TText& text) override {
        if (!ScopeDeclaration) {
            return;
        }
        if (InEnum) {
            CurrentEnum.CppName = text.Data;
        } else {
            if (NextScopeName == NAMESPACE) {
                InCompositeNamespace = true;
                LastScope += text.Data;
            } else {
                LastScope = text.Data;
            }
        }
        ScopeDeclaration = false;
    }

    void OnEnterScope(size_t /* offset */) {
        if (ScopeDeclaration) {
            // unnamed declaration or typedef
            ScopeDeclaration = false;
        }
        InCompositeNamespace = false;
        Scope.push_back(LastScope);
        LastScope.clear();
        //PrintScope();
    }

    /// @param offset: terminating curly brace position
    void OnLeaveScope(size_t offset) {
        if (Scope.empty()) {
            size_t contextOffsetBegin = (offset >= 256) ? offset - 256 : 0;
            std::string codeContext = std::string(Data + contextOffsetBegin, offset - contextOffsetBegin + 1);
            ythrow yexception() << "C++ source parse failed: unbalanced scope. Did you miss a closing '}' bracket? "
                "Context: enum " << NUtils::Quote(CurrentEnum.CppName) <<
                " in scope " << NUtils::Quote(TEnumParser::ScopeStr(CurrentEnum.Scope)) << ". Code context:\n... " <<
                codeContext << " ...";
        }
        Scope.pop_back();

        if (InEnum) {
            Y_ASSERT(offset > EnumPos);
            InEnum = false;
            try {
               ParseEnum(Data + EnumPos, offset - EnumPos + 1);
            } catch (...) {
                std::string ofFile;
                if (!SourceFileName.empty()) {
                    ofFile += " of file ";
                    ofFile += NUtils::Quote(SourceFileName);
                }
                ythrow yexception() << "Failed to parse enum " << CurrentEnum.CppName <<
                    " in scope " << TEnumParser::ScopeStr(CurrentEnum.Scope) << ofFile <<
                    "\n<C++ parser error message>: " << CurrentExceptionMessage();
            }
        }
        //PrintScope();
    }

    void ParseEnum(const char* data, size_t length) {
        TEnumContext enumContext(CurrentEnum);
        TMemoryInput in(data, length);
        TCppSaxParser parser(&enumContext);
        TransferData(&in, &parser);
        parser.Finish();
        //PrintEnum(CurrentEnum);
        Enums.push_back(CurrentEnum);
    }

    // Some debug stuff goes here
    static void PrintScope(const TScope& scope) {
        Cerr << "Current scope: " << TEnumParser::ScopeStr(scope) << Endl;
    }

    void PrintScope() {
        PrintScope(Scope);
    }

    void PrintEnum(const TEnum& en) {
        Cerr << "Enum within scope " << NUtils::Quote(TEnumParser::ScopeStr(en.Scope)) << Endl;
        for (const auto& item : en.Items) {
            Cerr << "    " << item.CppName;
            if (item.Value)
                Cerr << " = " << *item.Value;
            Cerr << Endl;
            for (const auto& value : item.Aliases) {
                Cerr << "        " << value << Endl;
            }
        }
    }

    void PrintEnums() {
        for (const auto& en : Enums)
            PrintEnum(en);
    }

public:
    TScope Scope;
    TEnums Enums;
private:
    const char* const Data;
    std::string SourceFileName;

    bool InEnum = false;
    bool ScopeDeclaration = false;
    bool InCompositeNamespace = false;
    std::string NextScopeName = BLOCK;
    std::string LastScope;
    size_t EnumPos = 0;
    TEnum CurrentEnum;
};

TEnumParser::TEnumParser(const std::string& fileName) {
    std::unique_ptr<IInputStream> hIn;
    IInputStream* in = nullptr;
    if (fileName != "-") {
        SourceFileName = fileName;
        hIn.reset(new TFileInput(fileName.c_str()));
        in = hIn.get();
    } else {
        in = &Cin;
    }

    std::string contents = in->ReadAll();
    Parse(contents.data(), contents.size());
}

TEnumParser::TEnumParser(const char* data, size_t length) {
    Parse(data, length);
}

TEnumParser::TEnumParser(IInputStream& in) {
    std::string contents = in.ReadAll();
    Parse(contents.data(), contents.size());
}

void TEnumParser::Parse(const char* dataIn, size_t lengthIn) {
    TMemoryInput mi(dataIn, lengthIn);

    std::string line;
    std::string result;

    while (mi.ReadLine(line)) {
        if (line.find("if (GetOwningArena() == other->GetOwningArena()) {") == std::string::npos) {
            result += line;
            result += "\n";
        }
    }

    const char* data = result.c_str();
    size_t length = result.length();

    const std::string_view span(data, length);
    const bool hasPragmaOnce = span.contains("#pragma once");
    const bool isProtobufHeader = span.contains("// Generated by the protocol buffer compiler");
    const bool isFlatbuffersHeader = span.contains("// automatically generated by the FlatBuffers compiler");
    Y_ENSURE(
        hasPragmaOnce || isProtobufHeader || isFlatbuffersHeader,
        "Serialization functions can be generated only for enums in header files. "
        "A valid header should either contain `#pragma once` or be an protobuf/flatbuf autogenerated header file. "
        "See SEARCH-975 for more information. "
    );
    TCppContext cppContext(data, SourceFileName);
    TMemoryInput in(data, length);
    TCppSaxParser parser(&cppContext);
    TransferData(&in, &parser);
    parser.Finish();
    // obtain result
    Enums = cppContext.Enums;
    if (!cppContext.Scope.empty()) {
        cppContext.PrintEnums();
        cppContext.PrintScope();
        ythrow yexception() << "Unbalanced scope, something is wrong with enum parser. ";
    }
}
