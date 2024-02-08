#pragma once

#include <util/generic/fwd.h>
#include <util/generic/strbuf.h>

namespace NLastGetopt {
    /**
     * Split text to multiple lines so that each line fits the given width.
     * Can work with UTF8, understands ANSI escape codes.
     *
     * @param indent will print this string after each newline.
     * @param lastLineLen output: will set to number of unicode codepoints in the last printed line.
     * @param hasParagraphs output: will set to true if there are two consecutive newlines in the text.
     */
    std::string Wrap(ui32 width, std::string_view text, std::string_view indent = "", size_t* lastLineLen = nullptr, bool* hasParagraphs = nullptr);
}
