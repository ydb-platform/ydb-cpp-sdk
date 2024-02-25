#pragma once

#include <util/system/align.h>
#include <util/system/compiler.h>
#include <util/stream/output.h>
#include <util/ysaveload.h>

#include "fluent_settings_helpers.h"

namespace NYdb {

enum class ES3Scheme {
    HTTP = 1 /* "http" */,
    HTTPS = 2 /* "https" */,
};

template <typename TDerived>
struct TS3Settings {
    using TSelf = TDerived;

    FLUENT_SETTING(TString, Endpoint);
    FLUENT_SETTING_DEFAULT(ES3Scheme, Scheme, ES3Scheme::HTTPS);
    FLUENT_SETTING(TString, Bucket);
    FLUENT_SETTING(TString, AccessKey);
    FLUENT_SETTING(TString, SecretKey);
};

} // namespace NYdb
