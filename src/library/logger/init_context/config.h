#pragma once

#include <src/library/logger/backend_creator.h>
#include <src/library/config/config.h>

class TLogBackendCreatorInitContextConfig : public ILogBackendCreator::IInitContext {
public:
    TLogBackendCreatorInitContextConfig(const NConfig::TConfig& config);
    virtual bool GetValue(std::string_view name, std::string& var) const override;
    virtual std::vector<THolder<IInitContext>> GetChildren(std::string_view name) const override;

private:
    const NConfig::TConfig& Config;
};
