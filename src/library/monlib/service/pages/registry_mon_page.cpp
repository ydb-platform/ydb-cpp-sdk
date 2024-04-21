#include "registry_mon_page.h"

#include <src/library/monlib/encode/text/text.h>
#include <ydb-cpp-sdk/library/monlib/encode/json/json.h>
#include <src/library/monlib/encode/prometheus/prometheus.h>
#include <src/library/monlib/encode/spack/spack_v1.h>
#include <src/library/monlib/service/format.h>

#include <ydb-cpp-sdk/library/string_utils/misc/misc.h>

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
            RegistryRawPtr_->Accept(TInstant::Zero(), encoder.get());

            encoder->Close();
        } else {
            THtmlMonPage::Output(request);
        }
    }

    void TMetricRegistryPage::OutputText(IOutputStream& out, NMonitoring::IMonHttpRequest&) {
        IMetricEncoderPtr encoder = NMonitoring::EncoderText(&out);
        RegistryRawPtr_->Accept(TInstant::Zero(), encoder.get());
    }

}
