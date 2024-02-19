#pragma once

#include "last_getopt_opts.h"

#include <library/cpp/colorizer/fwd.h>
#include <list>

namespace NLastGetopt {
    /**
* NLastGetopt::TOptsParser is an implementation of parsing
* argv/argv into TOptsParseResult by rules of TOpts.
*
* The class allows to make complicated handlers.
* Note, that if PERMUTE mode is on, then data, pointed by argv can be changed.
*/
    class TOptsParser {
        enum EIsOpt {
            EIO_NONE,  //is not an option name
            EIO_SDASH, //single-dashed ('-c') option name
            EIO_DDASH, //double-dashed ("--opt") option name
            EIO_PLUS,  //plus prefix ("+opt") option name
        };

    public:                 // TODO: make private
        const TOpts* Opts_; //rules of parsing

        // argc/argv pair
        size_t Argc_;
        const char** Argv_;

    private:
        //the storage of last unkown options. TODO: can be moved to local-method scope
        TCopyPtr<TOpt> TempCurrentOpt_;

    public:
        //storage of argv[0]
        std::string ProgramName_;

        //state of parsing:

        size_t Pos_; // current element withing argv
        size_t Sop_; // current char within arg
        bool Stopped_;
        bool GotMinusMinus_; //true if "--" have been seen in argv

    protected:
        const TOpt* CurrentOpt_;  // ptr on the last meeted option
        std::string_view CurrentValue_; // the value of the last met argument (corresponding to CurrentOpt_)

    private:
        typedef THashSet<const TOpt*> TdOptSet;
        TdOptSet OptsSeen_; //the set of options that have been met during parsing

        std::list<const TOpt*> OptsDefault_;

    private:
        void Init(const TOpts* options, int argc, const char* argv[]);
        void Init(const TOpts* options, int argc, char* argv[]);

        bool CommitEndOfOptions(size_t pos);
        bool Commit(const TOpt* currentOption, const std::string_view& currentValue, size_t pos, size_t sop);

        bool ParseShortOptArg(size_t pos);
        bool ParseOptArg(size_t pos);
        bool ParseOptParam(const TOpt* opt, size_t pos);
        bool ParseUnknownShortOptWithinArg(size_t pos, size_t sop);
        bool ParseShortOptWithinArg(size_t pos, size_t sop);
        bool ParseWithPermutation();

        bool DoNext();
        void Finish();

        EIsOpt IsOpt(const std::string_view& arg) const;

        void Swap(TOptsParser& that);

    public:
        TOptsParser(const TOpts* options, int argc, const char* argv[]) {
            Init(options, argc, argv);
        }

        TOptsParser(const TOpts* options, int argc, char* argv[]) {
            Init(options, argc, argv);
        }

        /// fetch next argument, false if no more arguments left
        bool Next();

        bool Seen(const TOpt* opt) const {
            return OptsSeen_.contains(opt);
        }

        bool Seen(std::string_view name) const {
            if (auto opt = Opts_->FindLongOption(name)) {
                return Seen(opt);
            } else {
                return false;
            }
        }

        bool Seen(char name) const {
            if (auto opt = Opts_->FindCharOption(name)) {
                return Seen(opt);
            } else {
                return false;
            }
        }

        const TOpt* CurOpt() const {
            return CurrentOpt_;
        }

        const char* CurVal() const {
            return CurrentValue_.data();
        }

        const std::string_view& CurValStr() const {
            return CurrentValue_;
        }

        std::string_view CurValOrOpt() const {
            std::string_view val(CurValStr());
            if (val.data() == nullptr && CurOpt()->HasOptionalValue())
                val = CurOpt()->GetOptionalValue();
            return val;
        }

        std::string_view CurValOrDef(bool useDef = true) const {
            std::string_view val(CurValOrOpt());
            if (val.data() == nullptr && useDef && CurOpt()->HasDefaultValue())
                val = CurOpt()->GetDefaultValue();
            return val;
        }

        // true if this option was actually specified by the user
        bool IsExplicit() const {
            return nullptr == CurrentOpt_ || !OptsSeen_.empty();
        }

        bool CurrentIs(const std::string& name) const {
            return CurOpt()->NameIs(name);
        }

        const std::string& ProgramName() const {
            return ProgramName_;
        }

        void PrintUsage(IOutputStream& os = Cout) const;

        void PrintUsage(IOutputStream& os, const NColorizer::TColors& colors) const;
    };
} //namespace NLastGetopt
