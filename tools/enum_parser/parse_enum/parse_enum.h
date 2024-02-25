#pragma once

#include <optional>

#include <util/stream/output.h>
#include <util/stream/input.h>
#include <util/stream/mem.h>
#include <util/string/strip.h>
#include <util/generic/string.h>
#include <util/generic/vector.h>

class TEnumParser {
public:

    struct TItem {
        std::optional<TString> Value;
        TString CppName;
        std::vector<TString> Aliases;
        TString CommentText;

        void Clear() {
            *this = TItem();
        }

        void NormalizeValue() {
            if (!Value)
                return;
            StripInPlace(*Value);
        }

    };

    // vector is to preserve declaration order
    typedef std::vector<TItem> TItems;

    typedef std::vector<TString> TScope;

    struct TEnum {
        TItems Items;
        TString CppName;
        TScope Scope;
        // enum or enum class
        bool EnumClass = false;
        bool BodyDetected = false;
        bool ForwardDeclaration = false;

        void Clear() {
            *this = TEnum();
        }
    };

    typedef std::vector<TEnum> TEnums;

    /// Parse results stored here
    TEnums Enums;

    /// Parse enums from file containing C++ code
    TEnumParser(const TString& fileName);

    /// Parse enums from memory buffer containing C++ code
    TEnumParser(const char* data, size_t length);

    /// Parse enums from input stream
    TEnumParser(IInputStream& in);

    static TString ScopeStr(const TScope& scope) {
        TString result;
        for (const TString& name : scope) {
            result += name;
            result += "::";
        }
        return result;
    }

private:
    void Parse(const char* data, size_t length);
protected:
    TString SourceFileName;
};
