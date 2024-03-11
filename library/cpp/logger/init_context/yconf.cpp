#include "yconf.h"

TLogBackendCreatorInitContextYConf::TLogBackendCreatorInitContextYConf(const TYandexConfig::Section& section)
    : Section(section)
{}

bool TLogBackendCreatorInitContextYConf::GetValue(std::string_view name, std::string& var) const {
    return Section.GetDirectives().GetValue(name, var);
}

std::vector<std::unique_ptr<ILogBackendCreator::IInitContext>> TLogBackendCreatorInitContextYConf::GetChildren(std::string_view name) const {
    std::vector<std::unique_ptr<IInitContext>> result;
    auto children = Section.GetAllChildren();
    for (auto range = children.equal_range(TCiString(name)); range.first != range.second; ++range.first) {
        result.emplace_back(std::make_unique<TLogBackendCreatorInitContextYConf>(*range.first->second));
    }
    return result;
}
