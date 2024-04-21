#include "wrap.h"

#include <src/library/colorizer/colors.h>

#include <src/util/stream/str.h>
#include <src/util/charset/utf8.h>

namespace NLastGetopt {
    std::string Wrap(ui32 width, std::string_view text, std::string_view indent, size_t* lastLineLen, bool* hasParagraphs) {
        if (width == 0) {
            return std::string(text);
        }

        if (width >= indent.size()) {
            width -= indent.size();
        }

        if (hasParagraphs) {
            *hasParagraphs = false;
        }

        std::string res;
        auto os = TStringOutput(res);

        const char* spaceBegin = text.begin();
        const char* wordBegin = text.begin();
        const char* wordEnd = text.begin();
        const char* end = text.end();

        size_t lenSoFar = 0;

        bool isPreParagraph = false;

        do {
            spaceBegin = wordBegin = wordEnd;

            while (wordBegin < end && *wordBegin == ' ') {
                wordBegin++;
            }

            if (wordBegin == end) {
                break;
            }

            wordEnd = wordBegin;

            while (wordEnd < end && *wordEnd != ' ' && *wordEnd != '\n') {
                wordEnd++;
            }

            auto spaces = std::string_view(spaceBegin, wordBegin);
            auto word = std::string_view(wordBegin, wordEnd);

            size_t spaceLen = spaces.size();

            size_t wordLen = 0;
            if (!GetNumberOfUTF8Chars(word.data(), word.size(), wordLen)) {
                wordLen = word.size(); // not a utf8 string -- just use its binary size
            }
            wordLen -= NColorizer::TotalAnsiEscapeCodeLen(word);

            // Empty word means we've found a bunch of whitespaces followed by newline.
            // We don't want to print trailing whitespaces.
            if (!word.empty()) {
                // We can't fit this word into the line -- insert additional line break.
                // We shouldn't insert line breaks if we're at the beginning of a line, hence `lenSoFar` check.
                if (lenSoFar && lenSoFar + spaceLen + wordLen > width) {
                    os << Endl << indent << word;
                    lenSoFar = wordLen;
                } else {
                    os << spaces << word;
                    lenSoFar += spaceLen + wordLen;
                }
                isPreParagraph = false;
            }

            if (wordEnd != end && *wordEnd == '\n') {
                os << Endl << indent;
                lenSoFar = 0;
                wordEnd++;
                if (hasParagraphs && isPreParagraph) {
                    *hasParagraphs = true;
                } else {
                    isPreParagraph = true;
                }
                continue;
            }
        } while (wordEnd < end);

        if (lastLineLen) {
            *lastLineLen = lenSoFar;
        }

        return res;
    }
}
