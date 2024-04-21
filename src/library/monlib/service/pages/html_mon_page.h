#pragma once

#include <ydb-cpp-sdk/library/monlib/service/pages/mon_page.h>

namespace NMonitoring {
    struct THtmlMonPage: public IMonPage {
        THtmlMonPage(const std::string& path,
                     const std::string& title = std::string(),
                     bool outputTableSorterJsCss = false)
            : IMonPage(path, title)
            , OutputTableSorterJsCss(outputTableSorterJsCss)
        {
        }

        void Output(NMonitoring::IMonHttpRequest& request) override;

        void NotFound(NMonitoring::IMonHttpRequest& request) const;
        void NoContent(NMonitoring::IMonHttpRequest& request) const;

        virtual void OutputContent(NMonitoring::IMonHttpRequest& request) = 0;

        bool OutputTableSorterJsCss;
    };

}
