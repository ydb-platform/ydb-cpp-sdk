#include "last_getopt_opt.h"

#include <ctype.h>

namespace NLastGetopt {
    static const std::string_view ExcludedShortNameChars = "= -\t\n";
    static const std::string_view ExcludedLongNameChars = "= \t\n";

    bool TOpt::NameIs(const std::string& name) const {
        for (const auto& next : LongNames_) {
            if (next == name)
                return true;
        }
        return false;
    }

    bool TOpt::CharIs(char c) const {
        for (auto next : Chars_) {
            if (next == c)
                return true;
        }
        return false;
    }

    char TOpt::GetChar() const {
        if (Chars_.empty())
            ythrow TConfException() << "no char for option " << this->ToShortString();
        return Chars_.at(0);
    }

    char TOpt::GetCharOr0() const {
        if (Chars_.empty())
            return 0;
        return GetChar();
    }

    std::string TOpt::GetName() const {
        if (LongNames_.empty())
            ythrow TConfException() << "no name for option " << this->ToShortString();
        return LongNames_.at(0);
    }

    bool TOpt::IsAllowedShortName(unsigned char c) {
        return isprint(c) && std::string_view::npos == ExcludedShortNameChars.find(c);
    }

    TOpt& TOpt::AddShortName(unsigned char c) {
        if (!IsAllowedShortName(c))
            throw TUsageException() << "option char '" << c << "' is not allowed";
        Chars_.push_back(c);
        return *this;
    }

    bool TOpt::IsAllowedLongName(const std::string& name, unsigned char* out) {
        for (size_t i = 0; i != name.size(); ++i) {
            const unsigned char c = name[i];
            if (!isprint(c) || std::string_view::npos != ExcludedLongNameChars.find(c)) {
                if (nullptr != out)
                    *out = c;
                return false;
            }
        }
        return true;
    }

    TOpt& TOpt::AddLongName(const std::string& name) {
        unsigned char c = 0;
        if (!IsAllowedLongName(name, &c))
            throw TUsageException() << "option char '" << c
                                     << "' in long '" << name << "' is not allowed";
        LongNames_.push_back(name);
        return *this;
    }

    namespace NPrivate {
        std::string OptToString(char c);

        std::string OptToString(const std::string& longOption);
    }

    std::string TOpt::ToShortString() const {
        if (!LongNames_.empty())
            return NPrivate::OptToString(LongNames_.front());
        if (!Chars_.empty())
            return NPrivate::OptToString(Chars_.front());
        return "?";
    }

    void TOpt::FireHandlers(const TOptsParser* parser) const {
        for (const auto& handler : Handlers_) {
            handler->HandleOpt(parser);
        }
    }

    TOpt& TOpt::IfPresentDisableCompletionFor(const TOpt& opt) {
        if (!opt.GetLongNames().empty()) {
            IfPresentDisableCompletionFor(opt.GetName());
        } else {
            IfPresentDisableCompletionFor(opt.GetChar());
        }
        return *this;
    }
}
