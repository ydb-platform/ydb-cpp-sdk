#include "misc.h"

#include <library/cpp/string_builder/string_builder.h>
#include <library/cpp/string_utils/escape/escape.h>

namespace NUtils {

std::string Quote(std::string_view s) {
    return TYdbStringBuilder() << "\"" << EscapeC(s) << "\"";
}

}