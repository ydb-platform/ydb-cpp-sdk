#include <ydb-cpp-sdk/library/monlib/service/pages/mon_page.h>

using namespace NMonitoring;

IMonPage::IMonPage(const std::string& path, const std::string& title)
    : Path(path)
    , Title(title)
{
    Y_ABORT_UNLESS(!Path.starts_with('/'));
    Y_ABORT_UNLESS(!Path.ends_with('/'));
}

void IMonPage::OutputNavBar(IOutputStream& out) {
    std::vector<const IMonPage*> parents;
    for (const IMonPage* p = this; p; p = p->Parent) {
        parents.push_back(p);
    }
    std::reverse(parents.begin(), parents.end());

    out << "<ol class='breadcrumb'>\n";

    std::string absolutePath;
    for (size_t i = 0; i < parents.size(); ++i) {
        const std::string& title = parents[i]->GetTitle();
        if (i == parents.size() - 1) {
            out << "<li>" << title << "</li>\n";
        } else {
            if (!absolutePath.ends_with('/')) {
                absolutePath += '/';
            }
            absolutePath += parents[i]->GetPath();
            out << "<li class='active'><a href='" << absolutePath << "'>" << title << "</a></li>\n";
        }
    }
    out << "</ol>\n";
}
