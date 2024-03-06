#pragma once

#include <library/cpp/logger/backend_creator.h>
#include <library/cpp/yconf/conf.h>

class TLogBackendCreatorInitContextYConf: public ILogBackendCreator::IInitContext {
public:
    TLogBackendCreatorInitContextYConf(const TYandexConfig::Section& section);
    virtual bool GetValue(std::string_view name, std::string& var) const override;
    virtual std::vector<std::unique_ptr<IInitContext>> GetChildren(std::string_view name) const override;
private:
    const TYandexConfig::Section& Section;
};
