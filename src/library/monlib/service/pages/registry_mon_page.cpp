#include "registry_mon_page.h"

#include <src/library/monlib/encode/text/text.h>
<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/library/monlib/encode/json/json.h>
=======
#include <src/library/monlib/encode/json/json.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/library/monlib/encode/json/json.h>
>>>>>>> origin/main
#include <src/library/monlib/encode/prometheus/prometheus.h>
#include <src/library/monlib/encode/spack/spack_v1.h>
#include <src/library/monlib/service/format.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/library/string_utils/misc/misc.h>
=======
#include <src/library/string_utils/misc/misc.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/library/string_utils/misc/misc.h>
>>>>>>> origin/main

namespace NMonitoring {
    void TMetricRegistryPage::Output(NMonitoring::IMonHttpRequest& request) {
        std::string_view pathInfo = request.GetPathInfo();
        const auto formatStr = NUtils::RNextTok(pathInfo, '/');
        auto& out = request.Output();

        if (!formatStr.empty()) {
            IMetricEncoderPtr encoder;
            std::string resp;

            if (formatStr == std::string_view("json")) {
                resp = HTTPOKJSON;
                encoder = NMonitoring::EncoderJson(&out);
            } else if (formatStr == std::string_view("spack")) {
                resp = HTTPOKSPACK;
                const auto compression = ParseCompression(request);
                encoder = NMonitoring::EncoderSpackV1(&out, ETimePrecision::SECONDS, compression);
            } else if (formatStr == std::string_view("prometheus")) {
                resp = HTTPOKPROMETHEUS;
                encoder = NMonitoring::EncoderPrometheus(&out);
            } else {
                ythrow yexception() << "unsupported metric encoding format: " << formatStr;
            }

            out.Write(resp);
            RegistryRawPtr_->Accept(TInstant::Zero(), encoder.Get());

            encoder->Close();
        } else {
            THtmlMonPage::Output(request);
        }
    }

    void TMetricRegistryPage::OutputText(IOutputStream& out, NMonitoring::IMonHttpRequest&) {
        IMetricEncoderPtr encoder = NMonitoring::EncoderText(&out);
        RegistryRawPtr_->Accept(TInstant::Zero(), encoder.Get());
    }

}
