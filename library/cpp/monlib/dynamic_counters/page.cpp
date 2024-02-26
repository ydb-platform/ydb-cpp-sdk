#include "page.h"
#include "encode.h"

#include <library/cpp/monlib/service/pages/templates.h>
#include <library/cpp/string_utils/quote/quote.h>

#include <util/string/split.h>
#include <util/system/tls.h>

using namespace NMonitoring;

namespace {
    Y_POD_STATIC_THREAD(TDynamicCounters*)
    currentCounters(nullptr);
}

TMaybe<EFormat> ParseFormat(std::string_view str) {
    if (str == std::string_view("json")) {
        return EFormat::JSON;
    } else if (str == std::string_view("spack")) {
        return EFormat::SPACK;
    } else if (str == std::string_view("prometheus")) {
        return EFormat::PROMETHEUS;
    } else {
        return Nothing();
    }
}

void TDynamicCountersPage::Output(NMonitoring::IMonHttpRequest& request) {
    if (OutputCallback) {
        OutputCallback();
    }

    std::string nameLabel("sensor");
    TCountableBase::EVisibility visibility{
        TCountableBase::EVisibility::Public
    };

    std::vector<std::string_view> parts;
    StringSplitter(request.GetPathInfo())
        .Split('/')
        .SkipEmpty()
        .Collect(&parts);

    TMaybe<EFormat> format = !parts.empty() ? ParseFormat(parts.back()) : Nothing();
    if (format) {
        parts.pop_back();
    }

    if (!parts.empty() && parts.back().starts_with(std::string_view("name_label="))) {
        std::vector<std::string> labels;
        StringSplitter(parts.back()).Split('=').SkipEmpty().Collect(&labels);
        if (labels.size() == 2U) {
            nameLabel = labels.back();
        }
        parts.pop_back();
   }

    if (!parts.empty() && parts.back() == std::string_view("private")) {
        visibility = TCountableBase::EVisibility::Private;
        parts.pop_back();
    }

    auto counters = Counters;

    for (const auto& escaped : parts) {
        const auto part = CGIUnescapeRet(escaped);

        std::vector<std::string> labels;
        StringSplitter(part).Split('=').SkipEmpty().Collect(&labels);

        if (labels.size() != 2U)
            return NotFound(request);

        if (const auto child = counters->FindSubgroup(
                labels.front(),
                labels.back())) {

            counters = child;
        } else {
            return HandleAbsentSubgroup(request);
        }
    }

    if (!format) {
        currentCounters = counters.Get();
        THtmlMonPage::Output(request);
        currentCounters = nullptr;
        return;
    }

    IOutputStream& out = request.Output();
    if (*format == EFormat::JSON) {
        out << HTTPOKJSON;
    } else if (*format == EFormat::SPACK) {
        out << HTTPOKSPACK;
    } else if (*format == EFormat::PROMETHEUS) {
        out << HTTPOKPROMETHEUS;
    } else {
        ythrow yexception() << "unsupported metric encoding format: " << *format;
    }

    auto encoder = CreateEncoder(&out, *format, nameLabel, visibility);
    counters->Accept(std::string(), std::string(), *encoder);
    out.Flush();
}

void TDynamicCountersPage::HandleAbsentSubgroup(IMonHttpRequest& request) {
    if (UnknownGroupPolicy == EUnknownGroupPolicy::Error) {
        NotFound(request);
    } else if (UnknownGroupPolicy == EUnknownGroupPolicy::Ignore) {
        NoContent(request);
    } else {
        Y_ABORT("Unsupported policy set");
    }
}

void TDynamicCountersPage::BeforePre(IMonHttpRequest& request) {
    IOutputStream& out = request.Output();
    HTML(out) {
        DIV() {
            out << "<a href='" << request.GetPath() << "/json'>Counters as JSON</a>";
            out << " for Solomon";
        }

        TAG(TH5) {
            out << "Counters subgroups";
        }
        UL() {
            currentCounters->EnumerateSubgroups([&](const std::string& name, const std::string& value) {
                LI() {
                    std::string pathPart = name + "=" + value;
                    Quote(pathPart, "");
                    out << "\n<a href='" << request.GetPath() << "/" << pathPart << "'>" << name << " " << value << "</a>";
                }
            });
        }

        TAG(TH4) {
            out << "Counters as text";
        }
    }
}

void TDynamicCountersPage::OutputText(IOutputStream& out, IMonHttpRequest&) {
    currentCounters->OutputPlainText(out);
}

void TDynamicCountersPage::SetUnknownGroupPolicy(EUnknownGroupPolicy value) {
    UnknownGroupPolicy = value;
}
