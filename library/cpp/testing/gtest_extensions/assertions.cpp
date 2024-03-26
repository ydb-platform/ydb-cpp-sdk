#include "assertions.h"

#include <util/string/builder.h>
#include <util/string/split.h>
#include <util/system/type_name.h>

namespace NGTest::NInternal {
    namespace {
        void FormatActual(const std::exception& err, const TBackTrace* bt, TYdbStringBuilder& out) {
            out << "an exception of type " << TypeName(err) << " "
                << "with message " << std::string(err.what()).Quote() << ".";
            if (bt) {
                out << "\n   Trace: ";
                for (auto& line: StringSplitter(bt->PrintToString()).Split('\n')) {
                    out << "          " << line.Token() << "\n";
                }
            }
        }

        void FormatActual(TYdbStringBuilder& out) {
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

        void FormatExpected(const char* statement, const char* type, const std::string& contains, TYdbStringBuilder& out) {
            out << "Expected: ";
            if (std::string_view(statement).size() > 80) {
                out << "statement";
            } else {
                out << statement;
            }
            out << " throws an exception of type " << type;

            if (!contains.empty()) {
                out << " with message containing " << contains.Quote();
            }

            out << ".";
        }
    }

    std::string FormatErrorWrongException(const char* statement, const char* type) {
        return FormatErrorWrongException(statement, type, "");
    }

    std::string FormatErrorWrongException(const char* statement, const char* type, std::string contains) {
        TYdbStringBuilder out;

        FormatExpected(statement, type, contains, out);
        out << "\n";
        FormatActual(out);

        return out;
    }

    std::string FormatErrorUnexpectedException(const char* statement) {
        TYdbStringBuilder out;

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

    bool ExceptionMessageContains(const std::exception& err, std::string contains) {
        return std::string_view(err.what()).find(contains) != std::string_view::npos;
    }
}
