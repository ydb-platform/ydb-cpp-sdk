#pragma once

#include <src/library/monlib/service/pages/resource_mon_page.h>

namespace NMonitoring {
    struct TTablesorterJsMonPage: public TResourceMonPage {
        TTablesorterJsMonPage()
            : TResourceMonPage("jquery.tablesorter.js", "jquery.tablesorter.js", JAVASCRIPT)
        {
        }
    };

}
