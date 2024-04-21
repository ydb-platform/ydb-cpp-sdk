#include "location.h"
#include "uri.h"

namespace NUri {
    static const ui64 URI_PARSE_FLAGS =
        (TFeature::FeaturesRecommended | TFeature::FeatureConvertHostIDN | TFeature::FeatureEncodeExtendedDelim | TFeature::FeatureEncodePercent) & ~TFeature::FeatureHashBangToEscapedFragment;

    std::string ResolveRedirectLocation(const std::string_view& baseUrl,
                                    const std::string_view& location) {
        TUri baseUri;
        TUri locationUri;

        // Parse base URL.
        if (baseUri.Parse(baseUrl, URI_PARSE_FLAGS) != NUri::TState::ParsedOK) {
            return "";
        }
        // Parse location with respect to the base URL.
        if (locationUri.Parse(location, baseUri, URI_PARSE_FLAGS) != NUri::TState::ParsedOK) {
            return "";
        }
        // Inherit fragment.
        if (locationUri.GetField(NUri::TField::FieldFragment).empty()) {
            NUri::TUriUpdate update(locationUri);
            update.Set(NUri::TField::FieldFragment, baseUri.GetField(NUri::TField::FieldFragment));
        }
        std::string res;
        locationUri.Print(res, NUri::TField::FlagAllFields);
        return res;
    }

}
