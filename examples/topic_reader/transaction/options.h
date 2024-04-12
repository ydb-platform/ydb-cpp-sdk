#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/library/logger/priority.h>
=======
#include <src/library/logger/priority.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#include <string>

struct TOptions {
    std::string Endpoint;
    std::string Database;
    std::string TopicPath;
    std::string ConsumerName;
    bool UseSecureConnection = false;
    std::string TablePath;
    ELogPriority LogPriority = TLOG_WARNING;

    TOptions(int argc, const char* argv[]);
};
