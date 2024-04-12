#pragma once

#include <src/util/system/defaults.h>
#include <src/library/http/misc/httpcodes.h>

enum ExtHttpCodes {
    // Custom
    HTTP_EXTENDED = 1000,
    HTTP_BAD_RESPONSE_HEADER = 1000,
    HTTP_CONNECTION_LOST = 1001,
    HTTP_BODY_TOO_LARGE = 1002,
    HTTP_ROBOTS_TXT_DISALLOW = 1003,
    HTTP_BAD_URL = 1004,
    HTTP_BAD_MIME = 1005,
    HTTP_DNS_FAILURE = 1006,
    HTTP_BAD_STATUS_CODE = 1007,
    HTTP_BAD_HEADER_STRING = 1008,
    HTTP_BAD_CHUNK = 1009,
    HTTP_CONNECT_FAILED = 1010,
    HTTP_FILTER_DISALLOW = 1011,
    HTTP_LOCAL_EIO = 1012,
    HTTP_BAD_CONTENT_LENGTH = 1013,
    HTTP_BAD_ENCODING = 1014,
    HTTP_LENGTH_UNKNOWN = 1015,
    HTTP_HEADER_EOF = 1016,
    HTTP_MESSAGE_EOF = 1017,
    HTTP_CHUNK_EOF = 1018,
    HTTP_PAST_EOF = 1019,
    HTTP_HEADER_TOO_LARGE = 1020,
    HTTP_URL_TOO_LARGE = 1021,
    HTTP_INTERRUPTED = 1022,
    HTTP_CUSTOM_NOT_MODIFIED = 1023,
    HTTP_BAD_CONTENT_ENCODING = 1024,
    HTTP_NO_RESOURCES = 1025,
    HTTP_FETCHER_SHUTDOWN = 1026,
    HTTP_CHUNK_TOO_LARGE = 1027,
    HTTP_SERVER_BUSY = 1028,
    HTTP_SERVICE_UNKNOWN = 1029,
    HTTP_PROXY_UNKNOWN = 1030,
    HTTP_PROXY_REQUEST_TIME_OUT = 1031,
    HTTP_PROXY_INTERNAL_ERROR = 1032,
    HTTP_PROXY_CONNECT_FAILED = 1033,
    HTTP_PROXY_CONNECTION_LOST = 1034,
    HTTP_PROXY_NO_PROXY = 1035,
    HTTP_PROXY_ERROR = 1036,
    HTTP_SSL_ERROR = 1037,
    HTTP_CACHED_COPY_NOT_FOUND = 1038,
    HTTP_TIMEDOUT_WHILE_BYTES_RECEIVING = 1039,
    HTTP_FETCHER_BAD_RESPONSE = 1040,
    HTTP_FETCHER_MB_ERROR = 1041,
    HTTP_SSL_CERT_ERROR = 1042,
    HTTP_FIREWALL_REJECT = 1043,
    HTTP_PROXY_REQUEST_CANCELED = 1051,

    // Custom (replace HTTP 200/304)
    EXT_HTTP_EXT_SUCCESS_BEGIN = 2000, // to check if code variable is in success interval
    EXT_HTTP_MIRRMOVE = 2000,
    EXT_HTTP_MANUAL_DELETE = 2001,
    EXT_HTTP_NOTUSED2 = 2002,
    EXT_HTTP_NOTUSED3 = 2003,
    EXT_HTTP_REFRESH = 2004,
    EXT_HTTP_NOINDEX = 2005,
    EXT_HTTP_BADCODES = 2006,
    EXT_HTTP_SITESTAT = 2007,
    EXT_HTTP_IOERROR = 2008,
    EXT_HTTP_BASEERROR = 2009,
    EXT_HTTP_PARSERROR = 2010,
    EXT_HTTP_BAD_CHARSET = 2011,
    EXT_HTTP_BAD_LANGUAGE = 2012,
    EXT_HTTP_NUMERERROR = 2013,
    EXT_HTTP_EMPTYDOC = 2014,
    EXT_HTTP_HUGEDOC = 2015,
    EXT_HTTP_LINKGARBAGE = 2016,
    EXT_HTTP_EXDUPLICATE = 2017,
    EXT_HTTP_FILTERED = 2018,
    EXT_HTTP_PARSERFAIL = 2019, // parser crashed (in this case image spider will redownload such document)
    EXT_HTTP_GZIPERROR = 2020,
    EXT_HTTP_CLEANPARAM = 2021,
    EXT_HTTP_MANUAL_DELETE_URL = 2022,
    EXT_HTTP_CUSTOM_PARTIAL_CONTENT = 2023,
    EXT_HTTP_EMPTY_RESPONSE = 2024,
    EXT_HTTP_REL_CANONICAL = 2025,

    EXT_HTTP_EXT_SUCCESS_END = 3000, // to check if code variable is in success interval
    EXT_HTTP_HOSTFILTER = 3001,
    EXT_HTTP_URLFILTER = 3002,
    EXT_HTTP_SUFFIXFILTER = 3003,
    EXT_HTTP_DOMAINFILTER = 3004,
    EXT_HTTP_EXTDOMAINFILTER = 3005,
    EXT_HTTP_PORTFILTER = 3006,
    EXT_HTTP_MIRROR = 3007,
    EXT_HTTP_DEEPDIR = 3008,
    EXT_HTTP_DUPDIRS = 3009,
    EXT_HTTP_REGEXP = 3010,
    EXT_HTTP_OLDDELETED = 3012,
    EXT_HTTP_PENALTY = 3013,
    EXT_HTTP_POLICY = 3015,
    EXT_HTTP_TOOOLD = 3016,
    EXT_HTTP_GARBAGE = 3017,
    EXT_HTTP_FOREIGN = 3018,
    EXT_HTTP_EXT_REGEXP = 3019,
    EXT_HTTP_HOPS = 3020,
    EXT_HTTP_SELRANK = 3021,
    EXT_HTTP_NOLINKS = 3022,
    EXT_HTTP_WRONGMULTILANG = 3023,
    EXT_HTTP_SOFTMIRRORS = 3024,
    EXT_HTTP_BIGLEVEL = 3025,

    // fast robot codes

    EXT_HTTP_FASTHOPS = 4000,
    EXT_HTTP_NODOC = 4001,

    EXT_HTTP_MAX
};

enum HttpFlags {
    // connection
    ShouldDisconnect = 1,
    ShouldRetry = 2,
    // UNUSED 4

    // indexer
    ShouldReindex = 8,
    ShouldDelete = 16,
    CheckLocation = 32,
    CheckLinks = 64,
    MarkSuspect = 128,
    // UNUSED 256
    // UNUSED 512
    MoveRedir = 1024,
    CanBeFake = 2048,
};

const size_t EXT_HTTP_CODE_MAX = 1 << 12;

static inline int Http2Status(int code) {
    extern ui16* http2status;
    return http2status[code & (EXT_HTTP_CODE_MAX - 1)];
}

std::string_view ExtHttpCodeStr(int code) noexcept;
