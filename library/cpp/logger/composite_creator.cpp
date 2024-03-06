#include "composite_creator.h"
#include "composite.h"
#include "uninitialized_creator.h"

std::unique_ptr<TLogBackend> TCompositeBackendCreator::DoCreateLogBackend() const {
    auto res = std::make_unique<TCompositeLogBackend>();
    for (const auto& child : Children) {
        res->AddLogBackend(child->CreateLogBackend());
    }
    return std::move(res);
}


TCompositeBackendCreator::TCompositeBackendCreator()
    : TLogBackendCreatorBase("composite")
{}

bool TCompositeBackendCreator::Init(const IInitContext& ctx) {
    for (const auto& child : ctx.GetChildren("SubLogger")) {
        Children.emplace_back(std::make_unique<TLogBackendCreatorUninitialized>());
        if (!Children.back()->Init(*child)) {
            return false;
        }
    }
    return true;
}

ILogBackendCreator::TFactory::TRegistrator<TCompositeBackendCreator> TCompositeBackendCreator::Registrar("composite");

void TCompositeBackendCreator::DoToJson(NJson::TJsonValue& value) const {
    for (const auto& child: Children) {
        child->ToJson(value["SubLogger"].AppendValue(NJson::JSON_MAP));
    }
}
