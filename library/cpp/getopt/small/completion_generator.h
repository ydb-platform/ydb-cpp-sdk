#pragma once

#include "completer.h"
#include "formatted_output.h"
#include "last_getopt_opts.h"
#include "modchooser.h"

#include <util/generic/variant.h>
#include <util/string/builder.h>

namespace NLastGetopt {
    class TCompletionGenerator {
    public:
        explicit TCompletionGenerator(const TModChooser* modChooser);
        explicit TCompletionGenerator(const TOpts* opts);
        virtual ~TCompletionGenerator() = default;

    public:
        virtual void Generate(std::string_view command, IOutputStream& stream) = 0;

    protected:
        std::variant<const TModChooser*, const TOpts*> Options_;
    };

    class TZshCompletionGenerator: public TCompletionGenerator {
    public:
        using TCompletionGenerator::TCompletionGenerator;

    public:
        void Generate(std::string_view command, IOutputStream& stream) override;

    private:
        static void GenerateModesCompletion(TFormattedOutput& out, const TModChooser& chooser, NComp::TCompleterManager& manager);
        static void GenerateOptsCompletion(TFormattedOutput& out, const TOpts& opts, NComp::TCompleterManager& manager);
        static void GenerateDefaultOptsCompletion(TFormattedOutput& out, NComp::TCompleterManager& manager);
        static void GenerateOptCompletion(TFormattedOutput& out, const TOpts& opts, const TOpt& opt, NComp::TCompleterManager& manager);
    };

    class TBashCompletionGenerator: public TCompletionGenerator {
    public:
        using TCompletionGenerator::TCompletionGenerator;

    public:
        void Generate(std::string_view command, IOutputStream& stream) override;

    private:
        static void GenerateModesCompletion(TFormattedOutput& out, const TModChooser& chooser, NComp::TCompleterManager& manager, size_t level);
        static void GenerateOptsCompletion(TFormattedOutput& out, const TOpts& opts, NComp::TCompleterManager& manager, size_t level);
        static void GenerateDefaultOptsCompletion(TFormattedOutput& out, NComp::TCompleterManager& manager);
    };

    namespace NEscaping {
        /// Escape ':', '-', '=', '[', ']' for use in zsh _arguments
        std::string Q(std::string_view string);
        std::string QQ(std::string_view string);

        /// Escape colons for use in zsh _alternative and _arguments
        std::string C(std::string_view string);
        std::string CC(std::string_view string);

        /// Simple escape for use in zsh single-quoted strings
        std::string S(std::string_view string);
        std::string SS(std::string_view string);

        /// Simple escape for use in bash single-quoted strings
        std::string B(std::string_view string);
        std::string BB(std::string_view string);
    }
}
