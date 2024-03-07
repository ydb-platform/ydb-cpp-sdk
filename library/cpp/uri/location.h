#pragma once

#include <string>

namespace NUri {
    /**
 * Resolve Location header according to https://tools.ietf.org/html/rfc7231#section-7.1.2
 *
 * @return  Resolved location's url or empty string in case of any error.
 */
    std::string ResolveRedirectLocation(const std::string_view& baseUrl, const std::string_view& location);

}
