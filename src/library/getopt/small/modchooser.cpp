#include "completer.h"
#include "completer_command.h"
#include "last_getopt.h"
#include "modchooser.h"

#include <src/library/colorizer/colors.h>

#include <src/util/folder/path.h>
#include <src/util/string/escape.h>

class PtrWrapper: public TMainClass {
public:
    explicit PtrWrapper(const TMainFunctionPtr& main)
        : Main(main)
    {
    }

    int operator()(const int argc, const char** argv) override {
        return Main(argc, argv);
    }

private:
    TMainFunctionPtr Main;
};

class PtrvWrapper: public TMainClass {
public:
    explicit PtrvWrapper(const TMainFunctionPtrV& main)
        : Main(main)
    {
    }

    int operator()(const int argc, const char** argv) override {
        std::vector<std::string> nargv(argv, argv + argc);
        return Main(nargv);
    }

private:
    TMainFunctionPtrV Main;
};

class ClassWrapper: public TMainClass {
public:
    explicit ClassWrapper(TMainClassV* main)
        : Main(main)
    {
    }

    int operator()(const int argc, const char** argv) override {
        std::vector<std::string> nargv(argv, argv + argc);
        return (*Main)(nargv);
    }

private:
    TMainClassV* Main;
};

TModChooser::TMode::TMode(const std::string& name, TMainClass* main, const std::string& descr, bool hidden, bool noCompletion)
    : Name(name)
    , Main(main)
    , Description(descr)
    , Hidden(hidden)
    , NoCompletion(noCompletion)
{
}

TModChooser::TModChooser()
    : ModesHelpOption("-?") // Default help option in last_getopt
    , VersionHandler(nullptr)
    , ShowSeparated(true)
    , SvnRevisionOptionDisabled(false)
    , PrintShortCommandInUsage(false)
{
}

TModChooser::~TModChooser() = default;

void TModChooser::AddMode(const std::string& mode, const TMainFunctionRawPtr func, const std::string& description, bool hidden, bool noCompletion) {
    AddMode(mode, TMainFunctionPtr(func), description, hidden, noCompletion);
}

void TModChooser::AddMode(const std::string& mode, const TMainFunctionRawPtrV func, const std::string& description, bool hidden, bool noCompletion) {
    AddMode(mode, TMainFunctionPtrV(func), description, hidden, noCompletion);
}

void TModChooser::AddMode(const std::string& mode, const TMainFunctionPtr func, const std::string& description, bool hidden, bool noCompletion) {
    Wrappers.push_back(MakeHolder<PtrWrapper>(func));
    AddMode(mode, Wrappers.back().Get(), description, hidden, noCompletion);
}

void TModChooser::AddMode(const std::string& mode, const TMainFunctionPtrV func, const std::string& description, bool hidden, bool noCompletion) {
    Wrappers.push_back(MakeHolder<PtrvWrapper>(func));
    AddMode(mode, Wrappers.back().Get(), description, hidden, noCompletion);
}

void TModChooser::AddMode(const std::string& mode, TMainClass* func, const std::string& description, bool hidden, bool noCompletion) {
    if (MapFindPtr(Modes, mode)) {
        ythrow yexception() << "TMode '" << mode << "' already exists in TModChooser.";
    }

    Modes[mode] = UnsortedModes.emplace_back(MakeHolder<TMode>(mode, func, description, hidden, noCompletion)).Get();
}

void TModChooser::AddMode(const std::string& mode, TMainClassV* func, const std::string& description, bool hidden, bool noCompletion) {
    Wrappers.push_back(MakeHolder<ClassWrapper>(func));
    AddMode(mode, Wrappers.back().Get(), description, hidden, noCompletion);
}

void TModChooser::AddGroupModeDescription(const std::string& description, bool hidden, bool noCompletion) {
    UnsortedModes.push_back(MakeHolder<TMode>(std::string(), nullptr, description.data(), hidden, noCompletion));
}

void TModChooser::SetDefaultMode(const std::string& mode) {
    DefaultMode = mode;
}

void TModChooser::AddAlias(const std::string& alias, const std::string& mode) {
    if (!MapFindPtr(Modes, mode)) {
        ythrow yexception() << "TMode '" << mode << "' not found in TModChooser.";
    }

    Modes[mode]->Aliases.push_back(alias);
    Modes[alias] = Modes[mode];
}

void TModChooser::SetDescription(const std::string& descr) {
    Description = descr;
}

void TModChooser::SetModesHelpOption(const std::string& helpOption) {
    ModesHelpOption = helpOption;
}

void TModChooser::SetVersionHandler(TVersionHandlerPtr handler) {
    VersionHandler = handler;
}

void TModChooser::SetSeparatedMode(bool separated) {
    ShowSeparated = separated;
}

void TModChooser::SetSeparationString(const std::string& str) {
    SeparationString = str;
}

void TModChooser::SetPrintShortCommandInUsage(bool printShortCommandInUsage = false) {
    PrintShortCommandInUsage = printShortCommandInUsage;
}

void TModChooser::DisableSvnRevisionOption() {
    SvnRevisionOptionDisabled = true;
}

void TModChooser::AddCompletions(std::string progName, const std::string& name, bool hidden, bool noCompletion) {
    if (CompletionsGenerator == nullptr) {
        CompletionsGenerator = NLastGetopt::MakeCompletionMod(this, std::move(progName), name);
        AddMode(name, CompletionsGenerator.Get(), "generate autocompletion files", hidden, noCompletion);
    }
}

int TModChooser::Run(const int argc, const char** argv) const {
    Y_ENSURE(argc, "Can't run TModChooser with empty list of arguments.");

    bool shiftArgs = true;
    std::string modeName;
    if (argc == 1) {
        if (DefaultMode.empty()) {
            PrintHelp(argv[0], HelpAlwaysToStdErr);
            return 0;
        } else {
            modeName = DefaultMode;
            shiftArgs = false;
        }
    } else {
        modeName = argv[1];
    }

    if (modeName == "-h" || modeName == "--help" || modeName == "-?") {
        PrintHelp(argv[0], HelpAlwaysToStdErr);
        return 0;
    }
    if (VersionHandler && (modeName == "-v" || modeName == "--version")) {
        VersionHandler();
        return 0;
    }
    if (!SvnRevisionOptionDisabled && modeName == "--svnrevision") {
        NLastGetopt::PrintVersionAndExit(nullptr);
    }

    auto modeIter = Modes.find(modeName);
    if (modeIter == Modes.end() && !DefaultMode.empty()) {
        modeIter = Modes.find(DefaultMode);
        shiftArgs = false;
    }

    if (modeIter == Modes.end()) {
        std::cerr << "Unknown mode " << NUtils::Quote(modeName) << "." << std::endl;
        PrintHelp(argv[0], true);
        return 1;
    }

    if (shiftArgs) {
        std::string firstArg;
        std::vector<const char*> nargv;
        nargv.reserve(argc);

        if (PrintShortCommandInUsage) {
            firstArg = modeIter->second->Name;
        } else {
            firstArg = argv[0] + std::string(" ") + modeIter->second->Name;
        }

        nargv.push_back(firstArg.data());

        for (int i = 2; i < argc; ++i) {
            nargv.push_back(argv[i]);
        }
        // According to the standard, "argv[argc] shall be a null pointer" (5.1.2.2.1).
        // http://www.open-std.org/JTC1/SC22/WG14/www/docs/n1336
        nargv.push_back(nullptr);

        return (*modeIter->second->Main)(nargv.size() - 1, nargv.data());
    } else {
        return (*modeIter->second->Main)(argc, argv);
    }
}

int TModChooser::Run(const std::vector<std::string>& argv) const {
    std::vector<const char*> nargv;
    nargv.reserve(argv.size() + 1);
    for (auto& arg : argv) {
        nargv.push_back(arg.c_str());
    }
    // According to the standard, "argv[argc] shall be a null pointer" (5.1.2.2.1).
    // http://www.open-std.org/JTC1/SC22/WG14/www/docs/n1336
    nargv.push_back(nullptr);

    return Run(nargv.size() - 1, nargv.data());
}

size_t TModChooser::TMode::CalculateFullNameLen() const {
    size_t len = Name.size();
    if (!Aliases.empty()) {
        len += 2;
        for (auto& alias : Aliases) {
            len += alias.size() + 1;
        }
    }
    return len;
}

std::string TModChooser::TMode::FormatFullName(size_t pad) const {
    TStringBuilder name;
    if (!Aliases.empty()) {
        name << "{";
    }

    name << NColorizer::StdErr().GreenColor();
    name << Name;
    name << NColorizer::StdErr().OldColor();

    if (!Aliases.empty()) {
        for (const auto& alias : Aliases) {
            name << "|" << NColorizer::StdErr().GreenColor() << alias << NColorizer::StdErr().OldColor();
        }
        name << "}";
    }

    auto len = CalculateFullNameLen();
    if (pad > len) {
        name << std::string(" ", pad - len);
    }

    return name;
}

void TModChooser::PrintHelp(const std::string& progName, bool toStdErr) const {
    auto baseName = TFsPath(progName).Basename();
    auto& out = toStdErr ? std::cerr : std::cout;
    const auto& colors = toStdErr ? NColorizer::StdErr() : NColorizer::StdOut();
    out << Description << std::endl << std::endl;
    out << colors.BoldColor() << "Usage" << colors.OldColor() << ": " << baseName << " MODE [MODE_OPTIONS]" << std::endl;
    out << std::endl;
    out << colors.BoldColor() << "Modes" << colors.OldColor() << ":" << std::endl;
    size_t maxModeLen = 0;
    for (const auto& [name, mode] : Modes) {
        if (name != mode->Name)
            continue;  // this is an alias
        maxModeLen = Max(maxModeLen, mode->CalculateFullNameLen());
    }

    if (ShowSeparated) {
        for (const auto& unsortedMode : UnsortedModes)
            if (!unsortedMode->Hidden) {
                if (unsortedMode->Name.size()) {
                    out << "  " << unsortedMode->FormatFullName(maxModeLen + 4) << unsortedMode->Description << std::endl;
                } else {
                    out << SeparationString << std::endl;
                    out << unsortedMode->Description << std::endl;
                }
            }
    } else {
        for (const auto& mode : Modes) {
            if (mode.first != mode.second->Name)
                continue;  // this is an alias

            if (!mode.second->Hidden) {
                out << "  " << mode.second->FormatFullName(maxModeLen + 4) << mode.second->Description << std::endl;
            }
        }
    }

    out << std::endl;
    out << "To get help for specific mode type '" << baseName << " MODE " << ModesHelpOption << "'" << std::endl;
    if (VersionHandler)
        out << "To print program version type '" << baseName << " --version'" << std::endl;
    if (!SvnRevisionOptionDisabled) {
        out << "To print svn revision type '" << baseName << " --svnrevision'" << std::endl;
    }
}

TVersionHandlerPtr TModChooser::GetVersionHandler() const {
    return VersionHandler;
}

bool TModChooser::IsSvnRevisionOptionDisabled() const {
    return SvnRevisionOptionDisabled;
}

int TMainClassArgs::Run(int argc, const char** argv) {
    return DoRun(NLastGetopt::TOptsParseResult(&GetOptions(), argc, argv));
}

const NLastGetopt::TOpts& TMainClassArgs::GetOptions() {
    if (!Opts_.has_value()) {
        Opts_ = NLastGetopt::TOpts();
        RegisterOptions(Opts_.value());
    }

    return Opts_.value();
}

void TMainClassArgs::RegisterOptions(NLastGetopt::TOpts& opts) {
    opts.AddHelpOption('h');
}

int TMainClassArgs::operator()(const int argc, const char** argv) {
    return Run(argc, argv);
}

int TMainClassModes::operator()(const int argc, const char** argv) {
    return Run(argc, argv);
}

int TMainClassModes::Run(int argc, const char** argv) {
    auto& chooser = GetSubModes();
    return chooser.Run(argc, argv);
}

const TModChooser& TMainClassModes::GetSubModes() {
    if (!Modes_.has_value()) {
        Modes_.emplace();
        RegisterModes(Modes_.value());
    }

    return Modes_.value();
}

void TMainClassModes::RegisterModes(TModChooser& modes) {
    modes.SetModesHelpOption("-h");
}
