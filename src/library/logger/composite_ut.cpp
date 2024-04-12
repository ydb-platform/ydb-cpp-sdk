#include "log.h"
#include <src/library/logger/init_context/config.h>
#include <src/library/logger/init_context/yconf.h>
#include <src/library/testing/unittest/registar.h>
#include <src/library/yconf/patcher/unstrict_config.h>
#include <src/util/stream/file.h>
#include <src/util/system/fs.h>

Y_UNIT_TEST_SUITE(TCompositeLogTest)
{
    std::vector<std::string> ReadLines(const std::string & filename) {
        std::vector<std::string> lines;
        TIFStream fin(filename);
        std::string line;
        while (fin.ReadLine(line)) {
            lines.push_back(std::move(line));
        }
        return lines;
    }

    void Clear(const std::string & filename) {
        NFs::Remove(filename + "1");
        NFs::Remove(filename + "2");
    }

    void DoTestComposite(const ILogBackendCreator::IInitContext& ctx, const std::string & filename) {
        Clear(filename);
        {
            TLog log;
            {
                auto creator = ILogBackendCreator::Create(ctx);
                log.ResetBackend(creator->CreateLogBackend());
                log.ReopenLog();
            }
            log.Write(TLOG_ERR, "first\n");
            log.Write(TLOG_DEBUG, "second\n");
        }
        auto data1 = ReadLines(filename + "1");
        auto data2 = ReadLines(filename + "2");
        UNIT_ASSERT_VALUES_EQUAL(data1.size(), 2);
        UNIT_ASSERT(data1[0] == "first");
        UNIT_ASSERT(data1[1] == "second");

        UNIT_ASSERT_VALUES_EQUAL(data2.size(), 1);
        UNIT_ASSERT(data2[0] == "first");
        Clear(filename);
    }

    Y_UNIT_TEST(TestCompositeConfig) {
        std::string s(R"(
{
    "LoggerType": "composite",
    "SubLogger":[
        {
            "LoggerType": "file",
            "Path": "config_log_1"
        }, {
            "LoggerType": "config_log_2",
            "LogLevel": "INFO"
        }
    ]
})");
        std::stringInput si(s);
        NConfig::TConfig cfg = NConfig::TConfig::FromJson(si);
        //Прогоняем конфигурацию через серализацию и десериализацию
        TLogBackendCreatorInitContextConfig ctx(cfg);
        std::string newCfg = ILogBackendCreator::Create(ctx)->AsJson().GetStringRobust();
        std::stringInput si2(newCfg);
        DoTestComposite(TLogBackendCreatorInitContextConfig(NConfig::TConfig::FromJson(si2)), "config_log_");

    }
    Y_UNIT_TEST(TestCompositeYConf) {
        constexpr const char* CONFIG = R"(
<Logger>
    LoggerType: composite
    <SubLogger>
        LoggerType: file
        Path: yconf_log_1
    </SubLogger>
    <SubLogger>
        LoggerType: yconf_log_2
        LogLevel: INFO
    </SubLogger>
</Logger>
)";
        TUnstrictConfig cfg;
        if (!cfg.ParseMemory(CONFIG)) {
            std::string errors;
            cfg.PrintErrors(errors);
            UNIT_ASSERT_C(false, errors);
        }
        TLogBackendCreatorInitContextYConf ctx(*cfg.GetFirstChild("Logger"));
        //Прогоняем конфигурацию через серализацию и десериализацию
        TUnstrictConfig newCfg;
        UNIT_ASSERT(newCfg.ParseJson(ILogBackendCreator::Create(ctx)->AsJson()));
        DoTestComposite(TLogBackendCreatorInitContextYConf(*newCfg.GetRootSection()), "yconf_log_");
    }
}
