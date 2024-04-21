#pragma once

#include <src/library/logger/backend_creator.h>
#include <src/library/yconf/conf.h>

class TLogBackendCreatorInitContextYConf: public ILogBackendCreator::IInitContext {
public:
    TLogBackendCreatorInitContextYConf(const TYandexConfig::Section& section);
    virtual bool GetValue(std::string_view name, std::string& var) const override;
    virtual std::vector<THolder<IInitContext>> GetChildren(std::string_view name) const override;
private:
    const TYandexConfig::Section& Section;
};
