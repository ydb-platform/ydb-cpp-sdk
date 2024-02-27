#pragma once

#include "pre_mon_page.h"

namespace NMonitoring {
    struct TVersionMonPage: public TPreMonPage {
        TVersionMonPage(const std::string& path = "ver", const std::string& title = "Version")
            : TPreMonPage(path, title)
        {
        }

        void OutputText(IOutputStream& out, NMonitoring::IMonHttpRequest&) override;
    };

}
