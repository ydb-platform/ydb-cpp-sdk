#pragma once

#include "modchooser.h"

namespace NLastGetopt {
    /// Create an option that generates completion.
    TOpt MakeCompletionOpt(const TOpts* opts, std::string command, std::string optName = "completion");

    /// Create a mode that generates completion.
    std::unique_ptr<TMainClassArgs> MakeCompletionMod(const TModChooser* modChooser, std::string command, std::string modName = "completion");
}
