#pragma once

#include <ydb-cpp-sdk/library/logger/backend.h>
#include <src/library/object_factory/object_factory.h>
#include <ydb-cpp-sdk/library/json/json_value.h>

#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <ydb-cpp-sdk/util/string/cast.h>

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
        virtual std::vector<THolder<IInitContext>> GetChildren(std::string_view name) const = 0;
    };

public:
    virtual ~ILogBackendCreator() = default;
    THolder<TLogBackend> CreateLogBackend() const;
    virtual bool Init(const IInitContext& ctx);

    NJson::TJsonValue AsJson() const;
    virtual void ToJson(NJson::TJsonValue& value) const = 0;

    static THolder<ILogBackendCreator> Create(const IInitContext& ctx);

private:
    virtual THolder<TLogBackend> DoCreateLogBackend() const = 0;
};

class TLogBackendCreatorBase: public ILogBackendCreator {
public:
    TLogBackendCreatorBase(const std::string& type);
    virtual void ToJson(NJson::TJsonValue& value) const override final;

protected:
    virtual void DoToJson(NJson::TJsonValue& value) const = 0;
    std::string Type;
};
