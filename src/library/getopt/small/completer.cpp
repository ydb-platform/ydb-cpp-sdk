#include "completer.h"

#include "completion_generator.h"

#include <ydb-cpp-sdk/util/string/builder.h>
#include <ydb-cpp-sdk/library/string_utils/misc/misc.h>

using NLastGetopt::NEscaping::Q;
using NLastGetopt::NEscaping::QQ;
using NLastGetopt::NEscaping::C;
using NLastGetopt::NEscaping::CC;
using NLastGetopt::NEscaping::S;
using NLastGetopt::NEscaping::SS;
using NLastGetopt::NEscaping::B;
using NLastGetopt::NEscaping::BB;

namespace NLastGetopt::NComp {
#define L out.Line()
#define I auto Y_GENERATE_UNIQUE_ID(indent) = out.Indent()

    TCompleterManager::TCompleterManager(std::string_view command)
        : Command_(command)
        , Id_(0)
    {
    }

    std::string_view TCompleterManager::GetCompleterID(const ICompleter* completer) {
        return Queue_.emplace_back(TStringBuilder() << "_" << Command_ << "__completer_" << ++Id_, completer).first;
    }

    void TCompleterManager::GenerateZsh(TFormattedOutput& out) {
        while (!Queue_.empty()) {
            auto[name, completer] = Queue_.back();
            Queue_.pop_back();

            L << "(( $+functions[" << name << "] )) ||";
            L << name << "() {";
            {
                I;
                completer->GenerateZsh(out, *this);
            }
            L << "}";
            L;
        }
    }

    class TAlternativeCompleter: public ICompleter {
    public:
        TAlternativeCompleter(std::vector<TAlternative>  alternatives)
            : Alternatives_(std::move(alternatives))
        {
        }

        void GenerateBash(TFormattedOutput& out) const override {
            for (auto& alternative: Alternatives_) {
                if (alternative.Completer != nullptr) {
                    alternative.Completer->GenerateBash(out);
                }
            }
        }

        std::string_view GenerateZshAction(TCompleterManager& manager) const override {
            return manager.GetCompleterID(this);
        }

        void GenerateZsh(TFormattedOutput& out, TCompleterManager& manager) const override {
            // We should use '_alternative' here, but it doesn't process escape codes in group descriptions,
            // so we dispatch alternatives ourselves.

            L << "local expl action";

            size_t i = 0;
            for (auto& alternative: Alternatives_) {
                auto tag = "alt-" + ToString(++i);
                auto action = alternative.Completer ? alternative.Completer->GenerateZshAction(manager) : std::string_view();

                L;

                if (action.empty()) {
                    L << "_message -e " << SS(tag) << " " << SS(alternative.Description);
                } else if (action.starts_with("((") && action.ends_with("))")) {
                    L << "action=" << action.substr(1, action.size() - 2);
                    L << "_describe -t " << SS(tag) << " " << SS(alternative.Description) << " action -M 'r:|[_-]=* r:|=*'";
                } else if (action.starts_with("(") && action.ends_with(")")) {
                    L << "action=" << action << "";
                    L << "_describe -t " << SS(tag) << " " << SS(alternative.Description) << " action -M 'r:|[_-]=* r:|=*'";
                } else if (action.starts_with(' ')) {
                    L << action.substr(1);
                } else {
                    L << "_description " << SS(tag) << " expl " << SS(alternative.Description);
                    std::string_view word, args;
                    NUtils::Split(action, word, args, ' ');
                    L << word << " \"${expl[@]}\" " << args;
                }
            }
        }

    private:
        std::vector<TAlternative> Alternatives_;
    };

    ICompleterPtr Alternative(std::vector<TAlternative> alternatives) {
        return std::make_shared<TAlternativeCompleter>(std::move(alternatives));
    }

    class TSimpleCompleter: public ICompleter {
    public:
        TSimpleCompleter(std::string bashCode, std::string action)
            : BashCode(std::move(bashCode))
            , Action(std::move(action))
        {
        }

        void GenerateBash(TFormattedOutput& out) const override {
            if (!BashCode.empty()) {
                L << BashCode;
            }
        }

        std::string_view GenerateZshAction(TCompleterManager&) const override {
            return Action;
        }

        void GenerateZsh(TFormattedOutput&, TCompleterManager&) const override {
            Y_ABORT("unreachable");
        }

    private:
        std::string BashCode;
        std::string Action;
    };

    ICompleterPtr Choice(std::vector<TChoice> choices) {
        TStringBuilder bash;  
        bash << "COMPREPLY+=( $(compgen -W '";
        std::string_view sep = "";
        for (auto& choice : choices) {
            bash << sep << B(choice.Choice);
            sep = " ";
        }
        bash << "' -- ${cur}) )";

        TStringBuilder action;
        action << "((";
        for (auto& choice: choices) {
            action << " " << SS(choice.Choice);
            if (!choice.Description.empty()) {{
                action << ":" << SS(choice.Description);
            }}
        }
        action << "))";
        return std::make_shared<TSimpleCompleter>(bash, action);
    }

    std::string Compgen(std::string_view flags) {
        return TStringBuilder() << "COMPREPLY+=( $(compgen " << flags << " -- ${cur}) )";
    }

    ICompleterPtr Default() {
        return std::make_shared<TSimpleCompleter>("", "_default");
    }

    ICompleterPtr File(std::string pattern) {
        if (!pattern.empty()) {
            pattern = " -g " + SS(pattern);
        }
        return std::make_shared<TSimpleCompleter>("", "_files" + pattern);
    }

    ICompleterPtr Directory() {
        return std::make_shared<TSimpleCompleter>("", "_files -/");
    }

    ICompleterPtr Host() {
        return std::make_shared<TSimpleCompleter>(Compgen("-A hostname"), "_hosts");
    }

    ICompleterPtr Pid() {
        return std::make_shared<TSimpleCompleter>("", "_pids");
    }

    ICompleterPtr User() {
        return std::make_shared<TSimpleCompleter>(Compgen("-A user"), "_users");
    }

    ICompleterPtr Group() {
        // For some reason, OSX freezes when trying to perform completion for groups.
        // You can try removing this ifdef and debugging it, but be prepared to force-shutdown your machine
        // (and possibly reinstall OSX if force-shutdown breaks anything).
#ifdef _darwin_
        return std::make_shared<TSimpleCompleter>("", "");
#else
        return std::make_shared<TSimpleCompleter>(Compgen("-A group"), "_groups");
#endif
    }

    ICompleterPtr Url() {
        return std::make_shared<TSimpleCompleter>("", "_urls");
    }

    ICompleterPtr Tty() {
        return std::make_shared<TSimpleCompleter>("", "_ttys");
    }

    ICompleterPtr NetInterface() {
        return std::make_shared<TSimpleCompleter>("", "_net_interfaces");
    }

    ICompleterPtr TimeZone() {
        return std::make_shared<TSimpleCompleter>("", "_time_zone");
    }

    ICompleterPtr Signal() {
        return std::make_shared<TSimpleCompleter>(Compgen("-A signal"), "_signals");
    }

    ICompleterPtr Domain() {
        return std::make_shared<TSimpleCompleter>("", "_domains");
    }

    namespace {
        TCustomCompleter* Head = nullptr;
        std::string_view SpecialFlag = "---CUSTOM-COMPLETION---";
    }

    void TCustomCompleter::FireCustomCompleter(int argc, const char** argv) {
        if (!argc) {
            return;
        }

        for (int i = 1; i < argc - 4; ++i) {
            if (SpecialFlag == argv[i]) {
                auto name = std::string_view(argv[i + 1]);
                auto curIdx = FromString<int>(argv[i + 2]);
                auto prefix = std::string_view(argv[i + 3]);
                auto suffix = std::string_view(argv[i + 4]);

                auto cur = std::string_view();
                if (0 <= curIdx && curIdx < i) {
                    cur = std::string_view(argv[curIdx]);
                }
                if (!cur.empty() && prefix.empty() && suffix.empty()) {
                    prefix = cur;  // bash does not send prefix and suffix
                }

                auto head = Head;
                while (head) {
                    if (head->GetUniqueName() == name) {
                        head->GenerateCompletions(i, argv, curIdx, cur, prefix, suffix);
                    }
                    head = head->Next_;
                }

                exit(0);
            }
        }
    }

    void TCustomCompleter::RegisterCustomCompleter(TCustomCompleter* completer) noexcept {
        Y_ABORT_UNLESS(completer);
        completer->Next_ = Head;
        Head = completer;
    }

    void TCustomCompleter::AddCompletion(std::string_view completion) {
        std::cout << completion << std::endl;  // this was easy =)
        // TODO: support option descriptions and messages
    }

    void TMultipartCustomCompleter::GenerateCompletions(int argc, const char** argv, int curIdx, std::string_view cur, std::string_view prefix, std::string_view suffix) {
        auto root = std::string_view();
        if (!Sep_.length() || prefix.find(Sep_) != std::string_view::npos) {
            auto tmp = std::string_view();
            NUtils::RSplit(prefix, root, tmp, Sep_);
        }

        if (!root.empty()) {
            std::cout << root << Sep_ << std::endl;
        } else {
            std::cout << std::endl;
        }

        std::cout << Sep_ << std::endl;

        GenerateCompletionParts(argc, argv, curIdx, cur, prefix, suffix, root);
    }

    class TLaunchSelf: public ICompleter {
    public:
        TLaunchSelf(TCustomCompleter* completer)
            : Completer_(completer)
        {
        }

        void GenerateBash(TFormattedOutput& out) const override {
            L << "IFS=$'\\n'";
            L << "COMPREPLY+=( $(compgen -W \"$(${words[@]} " << SpecialFlag << " " << Completer_->GetUniqueName() << " \"${cword}\" \"\" \"\" 2> /dev/null)\" -- ${cur}) )";
            L << "IFS=$' \\t\\n'";
        }

        std::string_view GenerateZshAction(TCompleterManager& manager) const override {
            return manager.GetCompleterID(this);
        }

        void GenerateZsh(TFormattedOutput& out, TCompleterManager&) const override {
            L << "compadd ${@} ${expl[@]} -- \"${(@f)$(${words_orig[@]} " << SpecialFlag << " " << Completer_->GetUniqueName() << " \"${current_orig}\" \"${prefix_orig}\" \"${suffix_orig}\" 2> /dev/null)}\"";
        }

    private:
        TCustomCompleter* Completer_;
    };

    ICompleterPtr LaunchSelf(TCustomCompleter& completer) {
        return std::make_shared<TLaunchSelf>(&completer);
    }

    class TLaunchSelfMultiPart: public ICompleter {
    public:
        TLaunchSelfMultiPart(TCustomCompleter* completer)
            : Completer_(completer)
        {
        }

        void GenerateBash(TFormattedOutput& out) const override {
            L << "IFS=$'\\n'";
            L << "items=( $(${words[@]} " << SpecialFlag << " " << Completer_->GetUniqueName() << " \"${cword}\" \"\" \"\" 2> /dev/null) )";
            L << "candidates=$(compgen -W \"${items[*]:1}\" -- \"$cur\")";
            L << "COMPREPLY+=( $candidates )";
            L << "[[ $candidates == *\"${items[1]}\" ]] && need_space=\"\"";
            L << "IFS=$' \\t\\n'";
        }

        std::string_view GenerateZshAction(TCompleterManager& manager) const override {
            return manager.GetCompleterID(this);
        }

        void GenerateZsh(TFormattedOutput& out, TCompleterManager&) const override {
            L << "local items=( \"${(@f)$(${words_orig[@]} " << SpecialFlag << " " << Completer_->GetUniqueName() << " \"${current_orig}\" \"${prefix_orig}\" \"${suffix_orig}\" 2> /dev/null)}\" )";
            L;
            L << "local rempat=${items[1]}";
            L << "shift items";
            L;
            L << "local sep=${items[1]}";
            L << "shift items";
            L;
            L << "local files=( ${items:#*\"${sep}\"} )";
            L << "local filenames=( ${files#\"${rempat}\"} )";
            L << "local dirs=( ${(M)items:#*\"${sep}\"} )";
            L << "local dirnames=( ${dirs#\"${rempat}\"} )";
            L;
            L << "local need_suf";
            L << "compset -S \"${sep}*\" || need_suf=\"1\"";
            L;
            L << "compadd ${@} ${expl[@]} -d filenames -- ${(q)files}";
            L << "compadd ${@} ${expl[@]} ${need_suf:+-S\"${sep}\"} -q -d dirnames -- ${(q)dirs%\"${sep}\"}";
        }

    private:
        TCustomCompleter* Completer_;
    };

    ICompleterPtr LaunchSelfMultiPart(TCustomCompleter& completer) {
        return std::make_shared<TLaunchSelfMultiPart>(&completer);
    }

#undef I
#undef L
}
