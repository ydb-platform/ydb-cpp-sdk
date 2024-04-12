#include "index_mon_page.h"

#include <src/util/generic/cast.h>
#include <src/util/string/ascii.h>

using namespace NMonitoring;

void TIndexMonPage::OutputIndexPage(IMonHttpRequest& request) {
    request.Output() << HTTPOKHTML;
    request.Output() << "<html>\n";
    OutputHead(request.Output());
    OutputBody(request);
    request.Output() << "</html>\n";
}

void TIndexMonPage::Output(IMonHttpRequest& request) {
    std::string_view pathInfo = request.GetPathInfo();
    if (pathInfo.empty() || pathInfo == std::string_view("/")) {
        OutputIndexPage(request);
        return;
    }

    Y_ABORT_UNLESS(pathInfo.starts_with('/'));

    TMonPagePtr found;
    // analogous to CGI PATH_INFO
    {
        std::lock_guard g(Mtx);
        std::string_view pathTmp = request.GetPathInfo();
        for (;;) {
            if (TPagesByPath::iterator i = PagesByPath.find(pathTmp); i != PagesByPath.end()) {
                found = *i->second;
                pathInfo = request.GetPathInfo().substr(pathTmp.size());
                Y_ABORT_UNLESS(pathInfo.empty() || pathInfo.starts_with('/'));
                break;
            }
            size_t slash = pathTmp.find_last_of('/');
            Y_ABORT_UNLESS(slash != std::string::npos);
            pathTmp = pathTmp.substr(0, slash);
            if (pathTmp.empty()) {
                break;
            }
        }
    }
    if (found) {
        THolder<IMonHttpRequest> child(request.MakeChild(found.Get(), std::string{pathInfo}));
        found->Output(*child);
    } else {
        request.Output() << HTTPNOTFOUND;
    }
}

void TIndexMonPage::OutputIndex(IOutputStream& out, bool pathEndsWithSlash) {
    std::lock_guard g(Mtx);
    for (auto& Page : Pages) {
        IMonPage* page = Page.Get();
        if (page->IsInIndex()) {
            std::string pathToDir = "";
            if (!pathEndsWithSlash) {
                pathToDir = this->GetPath() + "/";
            }
            out << "<a href='" << pathToDir << page->GetPath() << "'>" << page->GetTitle() << "</a><br/>\n";
        }
    }
}

void TIndexMonPage::Register(TMonPagePtr page) {
    std::lock_guard g(Mtx);
    if (auto [it, inserted] = PagesByPath.try_emplace("/" + page->GetPath()); inserted) {
        // new unique page just inserted, insert it to the end
        it->second = Pages.insert(Pages.end(), page);
    } else {
        // a page with the given path is already present, replace it with the new page
        *it->second = page;
    }
    page->Parent = this;
}

TIndexMonPage* TIndexMonPage::RegisterIndexPage(const std::string& path, const std::string& title) {
    std::lock_guard g(Mtx);
    TIndexMonPage* page = VerifyDynamicCast<TIndexMonPage*>(FindPage(path));
    if (page) {
        return page;
    }
    page = new TIndexMonPage(path, title);
    Register(page);
    return VerifyDynamicCast<TIndexMonPage*>(page);
}

IMonPage* TIndexMonPage::FindPage(const std::string& relativePath) {
    std::lock_guard g(Mtx);

    TPagesByPath::iterator i = PagesByPath.find("/" + relativePath);
    if (i == PagesByPath.end()) {
        return nullptr;
    } else {
        return i->second->Get();
    }
}

IMonPage* TIndexMonPage::FindPageByAbsolutePath(const std::string& absolutePath) {
    TIndexMonPage* page = this;
    std::string path = absolutePath;
    while (!path.empty()) {
        while (path.starts_with('/')) {
            path.erase(0, 1);
        }
        std::string tryPath = path;
        while (!tryPath.empty()) {
            IMonPage* found = page->FindPage(tryPath);
            if (found == nullptr) {
                size_t slash = tryPath.find_last_of('/');
                if (slash == std::string::npos) {
                    return nullptr;
                }
                tryPath.resize(slash);
            } else {
                if (tryPath.size() == path.size()) {
                    return found;
                }
                if (found->IsIndex()) {
                    path = path.substr(tryPath.size() + 1);
                    page = static_cast<TIndexMonPage*>(found);
                    break;
                } else {
                    return found;
                }
            }
        }
    }
    return nullptr;
}

TIndexMonPage* TIndexMonPage::FindIndexPage(const std::string& relativePath) {
    return VerifyDynamicCast<TIndexMonPage*>(FindPage(relativePath));
}

void TIndexMonPage::OutputCommonJsCss(IOutputStream& out) {
    out << "<link rel='stylesheet' href='/static/css/bootstrap.min.css'>\n";
    out << "<script language='javascript' type='text/javascript' src='/static/js/jquery.min.js'></script>\n";
    out << "<script language='javascript' type='text/javascript' src='/static/js/bootstrap.min.js'></script>\n";
}

void TIndexMonPage::OutputHead(IOutputStream& out) {
    out << "<head>\n";
    OutputCommonJsCss(out);
    out << "<title>" << Title << "</title>\n";
    out << "</head>\n";
}

void TIndexMonPage::OutputBody(IMonHttpRequest& req) {
    auto& out = req.Output();
    out << "<body>\n";

    // part of common navbar
    OutputNavBar(out);

    out << "<div class='container'>\n"
             << "<h2>" << Title << "</h2>\n";
    OutputIndex(out, req.GetPathInfo().ends_with('/'));
    out << "</div>\n"
        << "</body>\n";
}

void TIndexMonPage::SortPages() {
    std::lock_guard g(Mtx);
    Pages.sort([](const TMonPagePtr& a, const TMonPagePtr& b) {
        return AsciiCompareIgnoreCase(a->GetTitle(), b->GetTitle()) < 0;
    });
}

void TIndexMonPage::ClearPages() {
    std::lock_guard g(Mtx);
    Pages.clear();
    PagesByPath.clear();
}
