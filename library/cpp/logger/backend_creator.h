#pragma once

#include "backend.h"
#include <library/cpp/object_factory/object_factory.h>
#include <library/cpp/json/json_value.h>

#include <util/generic/ptr.h>
#include <util/string/cast.h>

class ILogBackendCreator {
public:
    using TFactory = NObjectFactory::TObjectFactory<ILogBackendCreator, std::string>;

    class IInitContext {
    public:
        template<class T>
        bool GetValue(std::string_view name, T& var) const {
            std::string tmp;
            if (!GetValue(name, tmp)) {
                return false;
            }
            var = FromString<T>(tmp);
            return true;
        }

        template<class T>
        T GetOrElse(std::string_view name, const T& def) const {
            T res;
            return GetValue(name, res) ? res : def;
        }

        virtual ~IInitContext() = default;
        virtual bool GetValue(std::string_view name, std::string& var) const = 0;
        virtual std::vector<std::unique_ptr<IInitContext>> GetChildren(std::string_view name) const = 0;
    };

public:
    virtual ~ILogBackendCreator() = default;
    std::unique_ptr<TLogBackend> CreateLogBackend() const;
    virtual bool Init(const IInitContext& ctx);

    NJson::TJsonValue AsJson() const;
    virtual void ToJson(NJson::TJsonValue& value) const = 0;

    static std::unique_ptr<ILogBackendCreator> Create(const IInitContext& ctx);

private:
    virtual std::unique_ptr<TLogBackend> DoCreateLogBackend() const = 0;
};

class TLogBackendCreatorBase: public ILogBackendCreator {
public:
    TLogBackendCreatorBase(const std::string& type);
    virtual void ToJson(NJson::TJsonValue& value) const override final;

protected:
    virtual void DoToJson(NJson::TJsonValue& value) const = 0;
    std::string Type;
};
