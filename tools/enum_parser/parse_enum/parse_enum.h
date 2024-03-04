#pragma once

#include <util/string/strip.h>
#include <util/generic/maybe.h>

#include <string>
#include <vector>

class TEnumParser {
public:

    struct TItem {
        std::optional<std::string> Value;
        std::string CppName;
        std::vector<std::string> Aliases;
        std::string CommentText;

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

    typedef std::vector<std::string> TScope;

    struct TEnum {
        TItems Items;
        std::string CppName;
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
    TEnumParser(const std::string& fileName);

    /// Parse enums from memory buffer containing C++ code
    TEnumParser(const char* data, size_t length);

    /// Parse enums from input stream
    TEnumParser(IInputStream& in);

    static std::string ScopeStr(const TScope& scope) {
        std::string result;
        for (const std::string& name : scope) {
            result += name;
            result += "::";
        }
        return result;
    }

private:
    void Parse(const char* data, size_t length);
protected:
    std::string SourceFileName;
};
