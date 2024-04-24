#pragma once

#include <ydb-cpp-sdk/util/string/builder.h>

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
        TStringBuilder& Line();

        /// Collect all lines into a stream.
        void Print(std::ostream& out);

    private:
        int IndentLevel_ = 0;
        std::vector<std::pair<int, TStringBuilder>> Lines_;
    };
}
