#include "formatted_output.h"

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/memory/tempbuf.h>
#include <ydb-cpp-sdk/util/generic/algorithm.h>
=======
#include <src/util/memory/tempbuf.h>
#include <src/util/generic/algorithm.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/memory/tempbuf.h>
#include <ydb-cpp-sdk/util/generic/algorithm.h>
>>>>>>> origin/main

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
