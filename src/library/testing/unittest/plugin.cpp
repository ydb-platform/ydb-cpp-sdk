#include "plugin.h"

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/singleton.h>
#include <ydb-cpp-sdk/util/generic/utility.h>
=======
#include <src/util/generic/singleton.h>
#include <src/util/generic/utility.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/singleton.h>
#include <ydb-cpp-sdk/util/generic/utility.h>
>>>>>>> origin/main
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

            void Register(TSimpleSharedPtr<IPlugin> plugin) {
                Plugins.emplace_back(std::move(plugin));
            }

            static TPlugins& Instance() {
                return *Singleton<TPlugins>();
            }

        private:
            std::vector<TSimpleSharedPtr<IPlugin>> Plugins;
        };
    } // anonymous namespace

    TPluginRegistrator::TPluginRegistrator(TSimpleSharedPtr<IPlugin> plugin) {
        TPlugins::Instance().Register(std::move(plugin));
    }

    void OnStartMain(int argc, char* argv[]) {
        TPlugins::Instance().OnStartMain(argc, argv);
    }

    void OnStopMain(int argc, char* argv[]) {
        TPlugins::Instance().OnStopMain(argc, argv);
    }

}