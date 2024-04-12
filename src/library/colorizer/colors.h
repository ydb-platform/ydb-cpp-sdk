#pragma once

#include "fwd.h"

#include <src/util/system/defaults.h>

#include <string>
#include <string_view>

#include <cstdio>

namespace NColorizer {
    /**
     * List of ECMA-48 colors.
     *
     * When printing elements of this enum via `operator<<`, `AutoColors()` (see below) function will be used
     * to produce colors, i.e. nothing will be printed to non-tty streams. When converting elements of this enum
     * via `ToString`, escape code is always returned.
     *
     * Note: as of now (2019-03), `ya make` strips out some escape codes from compiler output.
     * It also inserts `RESET` before each color code. See https://st.yandex-team.ru/DEVTOOLS-5269 for details.
     * For now, do not use `OLD`, `ST_*`, `FG_*` and `BG_*` in tools that run through `ya make`.
     *
     * Note: refrain from using black colors because there's high chance they'll not be visible on some terminals.
     * Default windows and ubuntu color schemes shows them as black letters on black background.
     * Also, white colors are barely visible in default OSX color scheme. Light black is usually fine though.
     */
    enum EAnsiCode: i8 {
        // Note: not using `GENERATE_ENUM_SERIALIZATION` because serialization generator depends on this library.

        /// Does not change anything.
        INVALID,

        /// Reset all styles and colors. Safe to use in `ya make` tools.
        RESET,

        /// Change style, don't change anything else.
        ST_LIGHT,
        ST_DARK,
        ST_NORMAL,

        /// Additional styles.
        ITALIC_ON,
        ITALIC_OFF,
        UNDERLINE_ON,
        UNDERLINE_OFF,

        /// Change foreground color, don't change anything else.
        FG_DEFAULT,
        FG_BLACK,
        FG_RED,
        FG_GREEN,
        FG_YELLOW,
        FG_BLUE,
        FG_MAGENTA,
        FG_CYAN,
        FG_WHITE,

        /// Change background color, don't change anything else.
        BG_DEFAULT,
        BG_BLACK,
        BG_RED,
        BG_GREEN,
        BG_YELLOW,
        BG_BLUE,
        BG_MAGENTA,
        BG_CYAN,
        BG_WHITE,

        /// Reset all styles and colors, then enable a (possibly light or dark) color. Safe to use in `ya make` tools.
        DEFAULT,
        BLACK,
        RED,
        GREEN,
        YELLOW,
        BLUE,
        MAGENTA,
        CYAN,
        WHITE,
        LIGHT_DEFAULT,
        LIGHT_BLACK,
        LIGHT_RED,
        LIGHT_GREEN,
        LIGHT_YELLOW,
        LIGHT_BLUE,
        LIGHT_MAGENTA,
        LIGHT_CYAN,
        LIGHT_WHITE,
        DARK_DEFAULT,
        DARK_BLACK,
        DARK_RED,
        DARK_GREEN,
        DARK_YELLOW,
        DARK_BLUE,
        DARK_MAGENTA,
        DARK_CYAN,
        DARK_WHITE,
    };

    /**
     * Produces escape codes or empty stringbuf depending on settings.
     * All color functions return zero-terminated stringbuf.
     */
    class TColors {
    public:
        static bool CalcIsTTY(FILE* file);

    public:
        explicit TColors(FILE* = stderr);
        explicit TColors(bool ontty);

        std::string_view Reset() const noexcept;

        std::string_view StyleLight() const noexcept;
        std::string_view StyleDark() const noexcept;
        std::string_view StyleNormal() const noexcept;

        std::string_view ItalicOn() const noexcept;
        std::string_view ItalicOff() const noexcept;
        std::string_view UnderlineOn() const noexcept;
        std::string_view UnderlineOff() const noexcept;

        std::string_view ForeDefault() const noexcept;
        std::string_view ForeBlack() const noexcept;
        std::string_view ForeRed() const noexcept;
        std::string_view ForeGreen() const noexcept;
        std::string_view ForeYellow() const noexcept;
        std::string_view ForeBlue() const noexcept;
        std::string_view ForeMagenta() const noexcept;
        std::string_view ForeCyan() const noexcept;
        std::string_view ForeWhite() const noexcept;

        std::string_view BackDefault() const noexcept;
        std::string_view BackBlack() const noexcept;
        std::string_view BackRed() const noexcept;
        std::string_view BackGreen() const noexcept;
        std::string_view BackYellow() const noexcept;
        std::string_view BackBlue() const noexcept;
        std::string_view BackMagenta() const noexcept;
        std::string_view BackCyan() const noexcept;
        std::string_view BackWhite() const noexcept;

        std::string_view Default() const noexcept;
        std::string_view Black() const noexcept;
        std::string_view Red() const noexcept;
        std::string_view Green() const noexcept;
        std::string_view Yellow() const noexcept;
        std::string_view Blue() const noexcept;
        std::string_view Magenta() const noexcept;
        std::string_view Cyan() const noexcept;
        std::string_view White() const noexcept;

        std::string_view LightDefault() const noexcept;
        std::string_view LightBlack() const noexcept;
        std::string_view LightRed() const noexcept;
        std::string_view LightGreen() const noexcept;
        std::string_view LightYellow() const noexcept;
        std::string_view LightBlue() const noexcept;
        std::string_view LightMagenta() const noexcept;
        std::string_view LightCyan() const noexcept;
        std::string_view LightWhite() const noexcept;

        std::string_view DarkDefault() const noexcept;
        std::string_view DarkBlack() const noexcept;
        std::string_view DarkRed() const noexcept;
        std::string_view DarkGreen() const noexcept;
        std::string_view DarkYellow() const noexcept;
        std::string_view DarkBlue() const noexcept;
        std::string_view DarkMagenta() const noexcept;
        std::string_view DarkCyan() const noexcept;
        std::string_view DarkWhite() const noexcept;

        /// Compatibility; prefer using methods without `Color` suffix in their names.
        /// Note: these behave differently from their un-suffixed counterparts.
        /// While functions declared above will reset colors completely, these will only reset foreground color and
        /// style, without changing the background color and underline/italic settings. Also, names of these functions
        /// don't conform with standard, e.g. `YellowColor` actually emits the `lite yellow` escape code.
        std::string_view OldColor() const noexcept;
        std::string_view BoldColor() const noexcept;
        std::string_view BlackColor() const noexcept;
        std::string_view BlueColor() const noexcept;
        std::string_view GreenColor() const noexcept;
        std::string_view CyanColor() const noexcept;
        std::string_view RedColor() const noexcept;
        std::string_view PurpleColor() const noexcept;
        std::string_view BrownColor() const noexcept;
        std::string_view LightGrayColor() const noexcept;
        std::string_view DarkGrayColor() const noexcept;
        std::string_view LightBlueColor() const noexcept;
        std::string_view LightGreenColor() const noexcept;
        std::string_view LightCyanColor() const noexcept;
        std::string_view LightRedColor() const noexcept;
        std::string_view LightPurpleColor() const noexcept;
        std::string_view YellowColor() const noexcept;
        std::string_view WhiteColor() const noexcept;

        inline bool IsTTY() const noexcept {
            return IsTTY_;
        }

        inline void SetIsTTY(bool value) noexcept {
            IsTTY_ = value;
        }

        inline void Enable() noexcept {
            SetIsTTY(true);
        }

        inline void Disable() noexcept {
            SetIsTTY(false);
        }

    private:
        bool IsTTY_;
    };

    /// Singletone `TColors` instances for stderr/stdout.
    TColors& StdErr();
    TColors& StdOut();

    /// Choose `TColors` depending on output stream. If passed stream is stderr/stdout, return a corresponding
    /// singletone. Otherwise, return a disabled singletone (which you can, but should *not* enable).
    TColors& AutoColors(std::ostream& os);

    /// Calculate total length of all ANSI escape codes in the text.
    size_t TotalAnsiEscapeCodeLen(std::string_view text);
}

std::string_view ToStringBuf(NColorizer::EAnsiCode x);
std::string ToString(NColorizer::EAnsiCode x);
