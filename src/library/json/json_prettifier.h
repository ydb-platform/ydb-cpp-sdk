#pragma once

#include <ydb-cpp-sdk/library/json/json_reader.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/ylimits.h>
=======
#include <src/util/generic/ylimits.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NJson {
    struct TJsonPrettifier {
        bool Unquote = false;
        ui8 Padding = 4;
        bool SingleQuotes = false;
        bool Compactify = false;
        bool Strict = false;
        bool NewUnquote = false; // use new unquote, may break old tests
        ui32 MaxPaddingLevel = Max<ui32>();

        static TJsonPrettifier Prettifier(bool unquote = false, ui8 padding = 4, bool singlequotes = false) {
            TJsonPrettifier p;
            p.Unquote = unquote;
            p.Padding = padding;
            p.SingleQuotes = singlequotes;
            return p;
        }

        static TJsonPrettifier Compactifier(bool unquote = false, bool singlequote = false) {
            TJsonPrettifier p;
            p.Unquote = unquote;
            p.Padding = 0;
            p.Compactify = true;
            p.SingleQuotes = singlequote;
            return p;
        }

        bool Prettify(std::string_view in, IOutputStream& out) const;

        std::string Prettify(std::string_view in) const;

        static bool MayUnquoteNew(std::string_view in);
        static bool MayUnquoteOld(std::string_view in);
    };

    inline std::string PrettifyJson(std::string_view in, bool unquote = false, ui8 padding = 4, bool sq = false) {
        return TJsonPrettifier::Prettifier(unquote, padding, sq).Prettify(in);
    }

    inline bool PrettifyJson(std::string_view in, IOutputStream& out, bool unquote = false, ui8 padding = 4, bool sq = false) {
        return TJsonPrettifier::Prettifier(unquote, padding, sq).Prettify(in, out);
    }

    inline bool CompactifyJson(std::string_view in, IOutputStream& out, bool unquote = false, bool sq = false) {
        return TJsonPrettifier::Compactifier(unquote, sq).Prettify(in, out);
    }

    inline std::string CompactifyJson(std::string_view in, bool unquote = false, bool sq = false) {
        return TJsonPrettifier::Compactifier(unquote, sq).Prettify(in);
    }

}
