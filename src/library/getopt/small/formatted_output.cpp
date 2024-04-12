#include "formatted_output.h"

#include <util/memory/tempbuf.h>
#include <util/generic/algorithm.h>

#include <iostream>

namespace NLastGetopt {

    TFormattedOutput::IndentGuard::IndentGuard(TFormattedOutput* output)
        : Output(output)
    {
        Output->IndentLevel_ += 2;
    }

    TFormattedOutput::IndentGuard::~IndentGuard() {
        Output->IndentLevel_ -= 2;
    }

    TFormattedOutput::IndentGuard TFormattedOutput::Indent() {
        return IndentGuard(this);
    }

    TStringBuilder& TFormattedOutput::Line() {
        return Lines_.emplace_back(IndentLevel_, TStringBuilder()).second;
    }

    void TFormattedOutput::Print(std::ostream& out) {
        for (auto&[indent, line] : Lines_) {
            if (indent && !line.empty()) {
                TTempBuf buf(indent);
                Fill(buf.Data(), buf.Data() + indent, ' ');
                out.write(buf.Data(), indent);
            }
            out << line;
            if (!std::string_view(line).ends_with('\n')) {
                out << std::endl;
            }
        }
    }
}
