#include "assertions.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/string/builder.h>
#include <ydb-cpp-sdk/util/string/escape.h>
#include <src/util/string/split.h>
#include <ydb-cpp-sdk/util/system/type_name.h>
=======
#include <src/util/string/builder.h>
#include <src/util/string/escape.h>
#include <src/util/string/split.h>
#include <src/util/system/type_name.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NGTest::NInternal {
    namespace {
        void FormatActual(const std::exception& err, const TBackTrace* bt, TStringBuilder& out) {
            out << "an exception of type " << TypeName(err) << " "
                << "with message " << NUtils::Quote(std::string(err.what())) << ".";
            if (bt) {
                out << "\n   Trace: ";
                for (auto& line: StringSplitter(bt->PrintToString()).Split('\n')) {
                    out << "          " << line.Token() << "\n";
                }
            }
        }

        void FormatActual(TStringBuilder& out) {
            out << "  Actual: it throws ";
            auto exceptionPtr = std::current_exception();
            if (exceptionPtr) {
                try {
                    std::rethrow_exception(exceptionPtr);
                } catch (const yexception& err) {
                    FormatActual(err, err.BackTrace(), out);
                    return;
                } catch (const std::exception& err) {
                    FormatActual(err, nullptr, out);
                    return;
                } catch (...) {
                    out << "an unknown exception.";
                    return;
                }
            }
            out << "nothing.";
        }

        void FormatExpected(const char* statement, const char* type, const std::string& contains, TStringBuilder& out) {
            out << "Expected: ";
            if (std::string_view(statement).size() > 80) {
                out << "statement";
            } else {
                out << statement;
            }
            out << " throws an exception of type " << type;

            if (!contains.empty()) {
                out << " with message containing " << NUtils::Quote(contains);
            }

            out << ".";
        }
    }

    std::string FormatErrorWrongException(const char* statement, const char* type) {
        return FormatErrorWrongException(statement, type, "");
    }

    std::string FormatErrorWrongException(const char* statement, const char* type, std::string contains) {
        TStringBuilder out;

        FormatExpected(statement, type, contains, out);
        out << "\n";
        FormatActual(out);

        return out;
    }

    std::string FormatErrorUnexpectedException(const char* statement) {
        TStringBuilder out;

        out << "Expected: ";
        if (std::string_view(statement).size() > 80) {
            out << "statement";
        } else {
            out << statement;
        }
        out << " doesn't throw an exception.\n  ";

        FormatActual(out);

        return out;
    }

    bool ExceptionMessageContains(const std::exception& err, std::string_view contains) {
        return std::string_view(err.what()).find(contains) != std::string_view::npos;
    }
}
