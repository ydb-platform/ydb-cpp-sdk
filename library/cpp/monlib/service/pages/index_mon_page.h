#pragma once

#include "mon_page.h"

#include <list>
#include <mutex>

namespace NMonitoring {
    struct TIndexMonPage: public IMonPage {
        std::mutex Mtx;
        using TPages = std::list<TMonPagePtr>;
        TPages Pages; // a list of pages to maintain specific order
        using TPagesByPath = THashMap<std::string, TPages::iterator>;
        TPagesByPath PagesByPath;

        TIndexMonPage(const std::string& path, const std::string& title)
            : IMonPage(path, title)
        {
        }

        ~TIndexMonPage() override {
        }

        bool IsIndex() const override {
            return true;
        }

        void Output(IMonHttpRequest& request) override;
        void OutputIndexPage(IMonHttpRequest& request);
        virtual void OutputIndex(IOutputStream& out, bool pathEndsWithSlash);
        virtual void OutputCommonJsCss(IOutputStream& out);
        void OutputHead(IOutputStream& out);
        void OutputBody(IMonHttpRequest& out);

        void Register(TMonPagePtr page);
        TIndexMonPage* RegisterIndexPage(const std::string& path, const std::string& title);

        IMonPage* FindPage(const std::string& relativePath);
        TIndexMonPage* FindIndexPage(const std::string& relativePath);
        IMonPage* FindPageByAbsolutePath(const std::string& absolutePath);

        void SortPages();
        void ClearPages();
    };

}
