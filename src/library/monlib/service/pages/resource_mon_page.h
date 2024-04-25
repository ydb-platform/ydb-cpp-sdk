#pragma once

#include <ydb-cpp-sdk/library/monlib/service/pages/mon_page.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/library/resource/resource.h>
=======
#include <src/library/resource/resource.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/library/resource/resource.h>
>>>>>>> origin/main

namespace NMonitoring {
    struct TResourceMonPage: public IMonPage {
    public:
        enum EResourceType {
            BINARY,
            TEXT,
            JSON,
            CSS,
            JAVASCRIPT,

            FONT_EOT,
            FONT_TTF,
            FONT_WOFF,
            FONT_WOFF2,

            PNG,
            SVG
        };

        TResourceMonPage(const std::string& path, const std::string& resourceName,
                         const EResourceType& resourceType = BINARY, const bool isCached = false)
            : IMonPage(path, "")
            , ResourceName(resourceName)
            , ResourceType(resourceType)
            , IsCached(isCached)
        {
        }

        void Output(NMonitoring::IMonHttpRequest& request) override;

        void NotFound(NMonitoring::IMonHttpRequest& request) const;

    private:
        std::string ResourceName;
        EResourceType ResourceType;
        bool IsCached;
    };

}
