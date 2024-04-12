#include "labels.h"

#include <src/util/stream/output.h>
#include <src/util/string/split.h>

static void OutputLabels(IOutputStream& out, const NMonitoring::ILabels& labels) {
    size_t i = 0;
    out << '{';
    for (const auto& label: labels) {
        if (i++ > 0) {
            out << std::string_view(", ");
        }
        out << label;
    }
    out << '}';
}

template <>
void Out<NMonitoring::ILabelsPtr>(IOutputStream& out, const NMonitoring::ILabelsPtr& labels) {
    OutputLabels(out, *labels);
}

template <>
void Out<NMonitoring::ILabels>(IOutputStream& out, const NMonitoring::ILabels& labels) {
    OutputLabels(out, labels);
}

template <>
void Out<NMonitoring::ILabel>(IOutputStream& out, const NMonitoring::ILabel& labels) {
    out << labels.Name() << "=" << labels.Value();
}

Y_MONLIB_DEFINE_LABELS_OUT(NMonitoring::TLabels);
Y_MONLIB_DEFINE_LABEL_OUT(NMonitoring::TLabel);

namespace NMonitoring {
    bool TryLoadLabelsFromString(std::string_view sb, ILabels& labels) {
        if (sb.empty()) {
            return false;
        }

        if (!sb.starts_with('{') || !sb.ends_with('}')) {
            return false;
        }

        sb.remove_prefix(1);
        sb.remove_suffix(1);

        if (sb.empty()) {
            return true;
        }

        bool ok = true;
        std::vector<std::pair<std::string_view, std::string_view>> rawLabels;
        StringSplitter(sb).SplitBySet(" ,").SkipEmpty().Consume([&] (std::string_view label) {
            std::string_view key, value;
            ok &= NUtils::TrySplit(label, key, value, '=');

            if (!ok) {
                return;
            }

            rawLabels.emplace_back(key, value);
        });

        if (!ok) {
            return false;
        }

        for (auto&& [k, v] : rawLabels) {
            labels.Add(k, v);
        }

        return true;
    }

    bool TryLoadLabelsFromString(IInputStream& is, ILabels& labels) {
        std::string str = is.ReadAll();
        return TryLoadLabelsFromString(str, labels);
    }

} // namespace NMonitoring
