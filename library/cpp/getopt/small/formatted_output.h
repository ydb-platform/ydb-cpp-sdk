#pragma once

#include <util/generic/algorithm.h>
#include <util/string/builder.h>
#include <util/memory/tempbuf.h>
#include <vector>

namespace NLastGetopt {
    /// Utility for printing indented lines. Used by completion generators.
    class TFormattedOutput {
    public:
        struct IndentGuard {
            explicit IndentGuard(TFormattedOutput* output);
            virtual ~IndentGuard();
            TFormattedOutput* Output;
        };

    public:
        /// Increase indentation and return a RAII object that'll decrease it back automatically.
        IndentGuard Indent();

        /// Append a new indented line to the stream.
        TYdbStringBuilder& Line();

        /// Collect all lines into a stream.
        void Print(IOutputStream& out);

    private:
        int IndentLevel_ = 0;
        std::vector<std::pair<int, TYdbStringBuilder>> Lines_;
    };
}
