#include "plugin.h"

#include <ydb-cpp-sdk/util/generic/singleton.h>
#include <ydb-cpp-sdk/util/generic/utility.h>
#include <vector>

namespace NUnitTest::NPlugin {
    namespace {
        class TPlugins {
        public:
            void OnStartMain(int argc, char* argv[]) const {
                for (const auto& plugin : Plugins) {
                    plugin->OnStartMain(argc, argv);
                }
            }

            void OnStopMain(int argc, char* argv[]) const {
                for (const auto& plugin : Plugins) {
                    plugin->OnStopMain(argc, argv);
                }
            }

            void Register(std::shared_ptr<IPlugin> plugin) {
                Plugins.emplace_back(std::move(plugin));
            }

            static TPlugins& Instance() {
                return *Singleton<TPlugins>();
            }

        private:
            std::vector<std::shared_ptr<IPlugin>> Plugins;
        };
    } // anonymous namespace

    TPluginRegistrator::TPluginRegistrator(std::shared_ptr<IPlugin> plugin) {
        TPlugins::Instance().Register(std::move(plugin));
    }

    void OnStartMain(int argc, char* argv[]) {
        TPlugins::Instance().OnStartMain(argc, argv);
    }

    void OnStopMain(int argc, char* argv[]) {
        TPlugins::Instance().OnStopMain(argc, argv);
    }

}