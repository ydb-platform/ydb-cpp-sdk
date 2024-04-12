#include "colors.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/generic/singleton.h>
=======
#include <src/util/stream/output.h>
#include <src/util/generic/singleton.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

#include <iostream>

#if defined(_unix_)
#include <unistd.h>
#endif

using namespace NColorizer;

namespace {
    constexpr std::string_view ToStringBufC(NColorizer::EAnsiCode x) {
        switch(x) {
            case RESET:
                return "\033[0m";

            case ST_LIGHT:
                return "\033[1m";
            case ST_DARK:
                return "\033[2m";
            case ST_NORMAL:
                return "\033[22m";

            case ITALIC_ON:
                return "\033[3m";
            case ITALIC_OFF:
                return "\033[23m";
            case UNDERLINE_ON:
                return "\033[4m";
            case UNDERLINE_OFF:
                return "\033[24m";

            case FG_DEFAULT:
                return "\033[39m";
            case FG_BLACK:
                return "\033[30m";
            case FG_RED:
                return "\033[31m";
            case FG_GREEN:
                return "\033[32m";
            case FG_YELLOW:
                return "\033[33m";
            case FG_BLUE:
                return "\033[34m";
            case FG_MAGENTA:
                return "\033[35m";
            case FG_CYAN:
                return "\033[36m";
            case FG_WHITE:
                return "\033[37m";

            case BG_DEFAULT:
                return "\033[49m";
            case BG_BLACK:
                return "\033[40m";
            case BG_RED:
                return "\033[41m";
            case BG_GREEN:
                return "\033[42m";
            case BG_YELLOW:
                return "\033[43m";
            case BG_BLUE:
                return "\033[44m";
            case BG_MAGENTA:
                return "\033[45m";
            case BG_CYAN:
                return "\033[46m";
            case BG_WHITE:
                return "\033[47m";

            // Note: the following codes are split into two escabe sequences because of how ya.make handles them.

            case DEFAULT:
                return "\033[0m\033[0;39m";
            case BLACK:
                return "\033[0m\033[0;30m";
            case RED:
                return "\033[0m\033[0;31m";
            case GREEN:
                return "\033[0m\033[0;32m";
            case YELLOW:
                return "\033[0m\033[0;33m";
            case BLUE:
                return "\033[0m\033[0;34m";
            case MAGENTA:
                return "\033[0m\033[0;35m";
            case CYAN:
                return "\033[0m\033[0;36m";
            case WHITE:
                return "\033[0m\033[0;37m";

            case LIGHT_DEFAULT:
                return "\033[0m\033[1;39m";
            case LIGHT_BLACK:
                return "\033[0m\033[1;30m";
            case LIGHT_RED:
                return "\033[0m\033[1;31m";
            case LIGHT_GREEN:
                return "\033[0m\033[1;32m";
            case LIGHT_YELLOW:
                return "\033[0m\033[1;33m";
            case LIGHT_BLUE:
                return "\033[0m\033[1;34m";
            case LIGHT_MAGENTA:
                return "\033[0m\033[1;35m";
            case LIGHT_CYAN:
                return "\033[0m\033[1;36m";
            case LIGHT_WHITE:
                return "\033[0m\033[1;37m";

            case DARK_DEFAULT:
                return "\033[0m\033[2;39m";
            case DARK_BLACK:
                return "\033[0m\033[2;30m";
            case DARK_RED:
                return "\033[0m\033[2;31m";
            case DARK_GREEN:
                return "\033[0m\033[2;32m";
            case DARK_YELLOW:
                return "\033[0m\033[2;33m";
            case DARK_BLUE:
                return "\033[0m\033[2;34m";
            case DARK_MAGENTA:
                return "\033[0m\033[2;35m";
            case DARK_CYAN:
                return "\033[0m\033[2;36m";
            case DARK_WHITE:
                return "\033[0m\033[2;37m";

            case INVALID:
            default:
                return "";
        }
    }
}

std::string_view ToStringBuf(NColorizer::EAnsiCode x) {
    return ToStringBufC(x);
}

std::string ToString(NColorizer::EAnsiCode x) {
    return std::string(ToStringBufC(x));
}

std::ostream& operator<<(std::ostream& os, NColorizer::EAnsiCode x) {
    if (AutoColors(os).IsTTY()) {
        os << ToStringBufC(x);
    }
    return os;
}

bool TColors::CalcIsTTY(FILE* file) {
    if (!std::string{std::getenv("ENFORCE_TTY") ? std::getenv("ENFORCE_TTY") : ""}.empty()) {
        return true;
    }

#if defined(_unix_)
    return isatty(fileno(file));
#else
    Y_UNUSED(file);
    return false;
#endif
}

TColors::TColors(FILE* f)
    : IsTTY_(true)
{
    SetIsTTY(CalcIsTTY(f));
}

TColors::TColors(bool ontty)
    : IsTTY_(true)
{
    SetIsTTY(ontty);
}

std::string_view TColors::Reset() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::RESET) : ToStringBufC(EAnsiCode::INVALID);
}

std::string_view TColors::StyleLight() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::ST_LIGHT) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::StyleDark() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::ST_DARK) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::StyleNormal() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::ST_NORMAL) : ToStringBufC(EAnsiCode::INVALID);
}

std::string_view TColors::ItalicOn() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::ITALIC_ON) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::ItalicOff() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::ITALIC_OFF) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::UnderlineOn() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::UNDERLINE_ON) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::UnderlineOff() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::UNDERLINE_OFF) : ToStringBufC(EAnsiCode::INVALID);
}

std::string_view TColors::ForeDefault() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::FG_DEFAULT) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::ForeBlack() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::FG_BLACK) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::ForeRed() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::FG_RED) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::ForeGreen() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::FG_GREEN) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::ForeYellow() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::FG_YELLOW) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::ForeBlue() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::FG_BLUE) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::ForeMagenta() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::FG_MAGENTA) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::ForeCyan() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::FG_CYAN) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::ForeWhite() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::FG_WHITE) : ToStringBufC(EAnsiCode::INVALID);
}

std::string_view TColors::BackDefault() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::BG_DEFAULT) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::BackBlack() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::BG_BLACK) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::BackRed() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::BG_RED) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::BackGreen() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::BG_GREEN) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::BackYellow() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::BG_YELLOW) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::BackBlue() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::BG_BLUE) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::BackMagenta() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::BG_MAGENTA) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::BackCyan() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::BG_CYAN) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::BackWhite() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::BG_WHITE) : ToStringBufC(EAnsiCode::INVALID);
}

std::string_view TColors::Default() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::DEFAULT) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::Black() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::BLACK) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::Red() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::RED) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::Green() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::GREEN) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::Yellow() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::YELLOW) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::Blue() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::BLUE) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::Magenta() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::MAGENTA) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::Cyan() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::CYAN) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::White() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::WHITE) : ToStringBufC(EAnsiCode::INVALID);
}

std::string_view TColors::LightDefault() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::LIGHT_DEFAULT) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::LightBlack() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::LIGHT_BLACK) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::LightRed() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::LIGHT_RED) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::LightGreen() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::LIGHT_GREEN) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::LightYellow() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::LIGHT_YELLOW) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::LightBlue() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::LIGHT_BLUE) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::LightMagenta() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::LIGHT_MAGENTA) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::LightCyan() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::LIGHT_CYAN) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::LightWhite() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::LIGHT_WHITE) : ToStringBufC(EAnsiCode::INVALID);
}

std::string_view TColors::DarkDefault() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::DARK_DEFAULT) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::DarkBlack() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::DARK_BLACK) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::DarkRed() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::DARK_RED) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::DarkGreen() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::DARK_GREEN) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::DarkYellow() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::DARK_YELLOW) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::DarkBlue() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::DARK_BLUE) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::DarkMagenta() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::DARK_MAGENTA) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::DarkCyan() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::DARK_CYAN) : ToStringBufC(EAnsiCode::INVALID);
}
std::string_view TColors::DarkWhite() const noexcept {
    return IsTTY() ? ToStringBufC(EAnsiCode::DARK_WHITE) : ToStringBufC(EAnsiCode::INVALID);
}

std::string_view TColors::OldColor() const noexcept {
    return IsTTY() ? "\033[22;39m" : "";
}

std::string_view TColors::BoldColor() const noexcept {
    return IsTTY() ? "\033[1m" : "";
}

std::string_view TColors::BlackColor() const noexcept {
    return IsTTY() ? "\033[22;30m" : "";
}

std::string_view TColors::BlueColor() const noexcept {
    return IsTTY() ? "\033[22;34m" : "";
}

std::string_view TColors::GreenColor() const noexcept {
    return IsTTY() ? "\033[22;32m" : "";
}

std::string_view TColors::CyanColor() const noexcept {
    return IsTTY() ? "\033[22;36m" : "";
}

std::string_view TColors::RedColor() const noexcept {
    return IsTTY() ? "\033[22;31m" : "";
}

std::string_view TColors::PurpleColor() const noexcept {
    return IsTTY() ? "\033[22;35m" : "";
}

std::string_view TColors::BrownColor() const noexcept {
    return IsTTY() ? "\033[22;33m" : "";
}

std::string_view TColors::LightGrayColor() const noexcept {
    return IsTTY() ? "\033[22;37m" : "";
}

std::string_view TColors::DarkGrayColor() const noexcept {
    return IsTTY() ? "\033[1;30m" : "";
}

std::string_view TColors::LightBlueColor() const noexcept {
    return IsTTY() ? "\033[1;34m" : "";
}

std::string_view TColors::LightGreenColor() const noexcept {
    return IsTTY() ? "\033[1;32m" : "";
}

std::string_view TColors::LightCyanColor() const noexcept {
    return IsTTY() ? "\033[1;36m" : "";
}

std::string_view TColors::LightRedColor() const noexcept {
    return IsTTY() ? "\033[1;31m" : "";
}

std::string_view TColors::LightPurpleColor() const noexcept {
    return IsTTY() ? "\033[1;35m" : "";
}

std::string_view TColors::YellowColor() const noexcept {
    return IsTTY() ? "\033[1;33m" : "";
}

std::string_view TColors::WhiteColor() const noexcept {
    return IsTTY() ? "\033[1;37m" : "";
}


namespace {
    class TStdErrColors: public TColors {
    public:
        TStdErrColors()
            : TColors(stderr)
        {
        }
    };

    class TStdOutColors: public TColors {
    public:
        TStdOutColors()
            : TColors(stdout)
        {
        }
    };

    class TDisabledColors: public TColors {
    public:
        TDisabledColors()
            : TColors(false)
        {
        }
    };
} // anonymous namespace

TColors& NColorizer::StdErr() {
    return *Singleton<TStdErrColors>();
}

TColors& NColorizer::StdOut() {
    return *Singleton<TStdOutColors>();
}

TColors& NColorizer::AutoColors(std::ostream& os) {
    if (&os == &std::cerr) {
        return StdErr();
    }
    if (&os == &std::cout) {
        return StdOut();
    }
    return *Singleton<TDisabledColors>();
}

size_t NColorizer::TotalAnsiEscapeCodeLen(std::string_view text) {
    enum {
        TEXT,
        BEFORE_CODE,
        IN_CODE,
    } state = TEXT;

    size_t totalLen = 0;
    size_t curLen = 0;

    for (auto it = text.begin(); it < text.end(); ++it) {
        switch (state) {
            case TEXT:
                if (*it == '\033') {
                    state = BEFORE_CODE;
                    curLen = 1;
                }
                break;
            case BEFORE_CODE:
                if (*it == '[') {
                    state = IN_CODE;
                    curLen++;
                } else {
                    state = TEXT;
                }
                break;
            case IN_CODE:
                if (*it == ';' || isdigit(*it)) {
                    curLen++;
                } else {
                    if (*it == 'm') {
                        totalLen += curLen + 1;
                    }
                    state = TEXT;
                }
                break;
        }
    }

    return totalLen;
}
