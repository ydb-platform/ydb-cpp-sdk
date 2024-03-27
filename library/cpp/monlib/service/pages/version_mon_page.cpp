#include <library/cpp/svnversion/svnversion.h>
#include <library/cpp/build_info/build_info.h>

#include "version_mon_page.h"

using namespace NMonitoring;

void TVersionMonPage::OutputText(IOutputStream& out, NMonitoring::IMonHttpRequest&) {
    const char* version = GetProgramSvnVersion();
    out << version;
    if (!std::string_view{version}.ends_with("\n"))
        out << "\n";
    out << GetBuildInfo() << "\n\n";
}
