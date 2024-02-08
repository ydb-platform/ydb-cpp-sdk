#pragma once

#include <util/generic/strbuf.h>


namespace NMonitoring {
namespace NPrometheus {

    //
    // Prometheus specific names and validation rules.
    //
    // See https://github.com/prometheus/docs/blob/master/content/docs/instrumenting/exposition_formats.md
    // and https://github.com/prometheus/common/blob/master/expfmt/text_parse.go
    //

    inline constexpr std::string_view BUCKET_SUFFIX = "_bucket";
    inline constexpr std::string_view COUNT_SUFFIX = "_count";
    inline constexpr std::string_view SUM_SUFFIX = "_sum";
    inline constexpr std::string_view MIN_SUFFIX = "_min";
    inline constexpr std::string_view MAX_SUFFIX = "_max";
    inline constexpr std::string_view LAST_SUFFIX = "_last";

    // Used for the label that defines the upper bound of a bucket of a
    // histogram ("le" -> "less or equal").
    inline constexpr std::string_view BUCKET_LABEL = "le";


    inline bool IsValidLabelNameStart(char ch) {
        return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
    }

    inline bool IsValidLabelNameContinuation(char ch) {
        return IsValidLabelNameStart(ch) || (ch >= '0' && ch <= '9');
    }

    inline bool IsValidMetricNameStart(char ch) {
        return IsValidLabelNameStart(ch) || ch == ':';
    }

    inline bool IsValidMetricNameContinuation(char ch) {
        return IsValidLabelNameContinuation(ch) || ch == ':';
    }

    inline bool IsSum(std::string_view name) {
        return name.EndsWith(SUM_SUFFIX);
    }

    inline bool IsCount(std::string_view name) {
        return name.EndsWith(COUNT_SUFFIX);
    }

    inline bool IsBucket(std::string_view name) {
        return name.EndsWith(BUCKET_SUFFIX);
    }

    inline std::string_view ToBaseName(std::string_view name) {
        if (IsBucket(name)) {
            return name.SubString(0, name.length() - BUCKET_SUFFIX.length());
        }
        if (IsCount(name)) {
            return name.SubString(0, name.length() - COUNT_SUFFIX.length());
        }
        if (IsSum(name)) {
            return name.SubString(0, name.length() - SUM_SUFFIX.length());
        }
        return name;
    }

} // namespace NPrometheus
} // namespace NMonitoring
