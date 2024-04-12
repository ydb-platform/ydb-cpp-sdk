#pragma once

#include <src/library/monlib/service/pages/resource_mon_page.h>

namespace NMonitoring {
    struct TTablesorterCssMonPage: public TResourceMonPage {
        TTablesorterCssMonPage()
            : TResourceMonPage("jquery.tablesorter.css", "jquery.tablesorter.css", CSS)
        {
        }
    };

}
