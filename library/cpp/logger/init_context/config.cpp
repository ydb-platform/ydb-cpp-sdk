#include "config.h"

TLogBackendCreatorInitContextConfig::TLogBackendCreatorInitContextConfig(const NConfig::TConfig& config)
    : Config(config)
{}

bool TLogBackendCreatorInitContextConfig::GetValue(std::string_view name, std::string& var) const {
    if (Config.Has(name)) {
        var = Config[name].Get<std::string>();
        return true;
    }
    return false;
}

std::vector<std::unique_ptr<ILogBackendCreator::IInitContext>> TLogBackendCreatorInitContextConfig::GetChildren(std::string_view name) const {
    std::vector<std::unique_ptr<IInitContext>> result;
    const NConfig::TConfig& child = Config[name];
    if (child.IsA<NConfig::TArray>()) {
        for (const auto& i: child.Get<NConfig::TArray>()) {
            result.emplace_back(std::make_unique<TLogBackendCreatorInitContextConfig>(i));
        }
    } else if (!child.IsNull()) {
        result.emplace_back(std::make_unique<TLogBackendCreatorInitContextConfig>(child));
    }
    return result;
}
