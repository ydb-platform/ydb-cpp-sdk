#include "httpcodes.h"

std::string_view HttpCodeStrEx(int code) noexcept {
    switch (code) {
        case HTTP_CONTINUE:
            return std::string_view("100 Continue");
        case HTTP_SWITCHING_PROTOCOLS:
            return std::string_view("101 Switching protocols");
        case HTTP_PROCESSING:
            return std::string_view("102 Processing");
        case HTTP_EARLY_HINTS:
            return std::string_view ("103 Early Hints");

        case HTTP_OK:
            return std::string_view("200 Ok");
        case HTTP_CREATED:
            return std::string_view("201 Created");
        case HTTP_ACCEPTED:
            return std::string_view("202 Accepted");
        case HTTP_NON_AUTHORITATIVE_INFORMATION:
            return std::string_view("203 None authoritative information");
        case HTTP_NO_CONTENT:
            return std::string_view("204 No content");
        case HTTP_RESET_CONTENT:
            return std::string_view("205 Reset content");
        case HTTP_PARTIAL_CONTENT:
            return std::string_view("206 Partial content");
        case HTTP_MULTI_STATUS:
            return std::string_view("207 Multi status");
        case HTTP_ALREADY_REPORTED:
            return std::string_view("208 Already reported");
        case HTTP_IM_USED:
            return std::string_view("226 IM used");

        case HTTP_MULTIPLE_CHOICES:
            return std::string_view("300 Multiple choices");
        case HTTP_MOVED_PERMANENTLY:
            return std::string_view("301 Moved permanently");
        case HTTP_FOUND:
            return std::string_view("302 Moved temporarily");
        case HTTP_SEE_OTHER:
            return std::string_view("303 See other");
        case HTTP_NOT_MODIFIED:
            return std::string_view("304 Not modified");
        case HTTP_USE_PROXY:
            return std::string_view("305 Use proxy");
        case HTTP_TEMPORARY_REDIRECT:
            return std::string_view("307 Temporarily redirect");
        case HTTP_PERMANENT_REDIRECT:
            return std::string_view("308 Permanent redirect");

        case HTTP_BAD_REQUEST:
            return std::string_view("400 Bad request");
        case HTTP_UNAUTHORIZED:
            return std::string_view("401 Unauthorized");
        case HTTP_PAYMENT_REQUIRED:
            return std::string_view("402 Payment required");
        case HTTP_FORBIDDEN:
            return std::string_view("403 Forbidden");
        case HTTP_NOT_FOUND:
            return std::string_view("404 Not found");
        case HTTP_METHOD_NOT_ALLOWED:
            return std::string_view("405 Method not allowed");
        case HTTP_NOT_ACCEPTABLE:
            return std::string_view("406 Not acceptable");
        case HTTP_PROXY_AUTHENTICATION_REQUIRED:
            return std::string_view("407 Proxy Authentication required");
        case HTTP_REQUEST_TIME_OUT:
            return std::string_view("408 Request time out");
        case HTTP_CONFLICT:
            return std::string_view("409 Conflict");
        case HTTP_GONE:
            return std::string_view("410 Gone");
        case HTTP_LENGTH_REQUIRED:
            return std::string_view("411 Length required");
        case HTTP_PRECONDITION_FAILED:
            return std::string_view("412 Precondition failed");
        case HTTP_REQUEST_ENTITY_TOO_LARGE:
            return std::string_view("413 Request entity too large");
        case HTTP_REQUEST_URI_TOO_LARGE:
            return std::string_view("414 Request uri too large");
        case HTTP_UNSUPPORTED_MEDIA_TYPE:
            return std::string_view("415 Unsupported media type");
        case HTTP_REQUESTED_RANGE_NOT_SATISFIABLE:
            return std::string_view("416 Requested Range Not Satisfiable");
        case HTTP_EXPECTATION_FAILED:
            return std::string_view("417 Expectation Failed");
        case HTTP_I_AM_A_TEAPOT:
            return std::string_view("418 I Am A Teapot");
        case HTTP_AUTHENTICATION_TIMEOUT:
            return std::string_view("419 Authentication Timeout");
        case HTTP_MISDIRECTED_REQUEST:
            return std::string_view("421 Misdirected Request");
        case HTTP_UNPROCESSABLE_ENTITY:
            return std::string_view("422 Unprocessable Entity");
        case HTTP_LOCKED:
            return std::string_view("423 Locked");
        case HTTP_FAILED_DEPENDENCY:
            return std::string_view("424 Failed Dependency");
        case HTTP_UNORDERED_COLLECTION:
            return std::string_view("425 Unordered Collection");
        case HTTP_UPGRADE_REQUIRED:
            return std::string_view("426 Upgrade Required");
        case HTTP_PRECONDITION_REQUIRED:
            return std::string_view("428 Precondition Required");
        case HTTP_TOO_MANY_REQUESTS:
            return std::string_view("429 Too Many Requests");
        case HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE:
            return std::string_view("431 Request Header Fields Too Large");
        case HTTP_UNAVAILABLE_FOR_LEGAL_REASONS:
            return std::string_view("451 Unavailable For Legal Reason");

        case HTTP_INTERNAL_SERVER_ERROR:
            return std::string_view("500 Internal server error");
        case HTTP_NOT_IMPLEMENTED:
            return std::string_view("501 Not implemented");
        case HTTP_BAD_GATEWAY:
            return std::string_view("502 Bad gateway");
        case HTTP_SERVICE_UNAVAILABLE:
            return std::string_view("503 Service unavailable");
        case HTTP_GATEWAY_TIME_OUT:
            return std::string_view("504 Gateway time out");
        case HTTP_HTTP_VERSION_NOT_SUPPORTED:
            return std::string_view("505 HTTP version not supported");
        case HTTP_VARIANT_ALSO_NEGOTIATES:
            return std::string_view("506 Variant also negotiates");
        case HTTP_INSUFFICIENT_STORAGE:
            return std::string_view("507 Insufficient storage");
        case HTTP_LOOP_DETECTED:
            return std::string_view("508 Loop Detected");
        case HTTP_BANDWIDTH_LIMIT_EXCEEDED:
            return std::string_view("509 Bandwidth Limit Exceeded");
        case HTTP_NOT_EXTENDED:
            return std::string_view("510 Not Extended");
        case HTTP_NETWORK_AUTHENTICATION_REQUIRED:
            return std::string_view("511 Network Authentication Required");
        case HTTP_UNASSIGNED_512:
            return std::string_view("512 Unassigned");

        default:
            return std::string_view("000 Unknown HTTP code");
    }
}
