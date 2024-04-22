#include "json_prettifier.h"

#include <ydb-cpp-sdk/util/generic/algorithm.h>
#include <src/util/memory/pool.h>
#include <ydb-cpp-sdk/util/stream/str.h>
#include <src/util/string/util.h>

#include <src/library/string_utils/relaxed_escaper/relaxed_escaper.h>

namespace NJson {
    struct TRewritableOut {
        IOutputStream& Slave;

        char Last = 0;
        bool Dirty = false;

        TRewritableOut(IOutputStream& sl)
            : Slave(sl)
        {
        }

        template <typename T>
        void Write(const T& t) {
            Flush();
            Slave << t;
        }

        void Hold(char c) {
            if (Dirty)
                Flush();
            Last = c;
            Dirty = true;
        }

        void Flush() {
            if (Dirty) {
                Slave << Last;
                Dirty = false;
            }
        }

        void Revert() {
            Dirty = false;
        }
    };

    struct TSpaces {
        char S[256];

        TSpaces() {
            memset(&S, ' ', sizeof(S));
        }

        std::string_view Get(ui8 sz) const {
            return std::string_view(S, sz);
        }
    };

    bool TJsonPrettifier::MayUnquoteNew(std::string_view s) {
        static str_spn alpha("a-zA-Z_@$", true);
        static str_spn alnum("a-zA-Z_@$0-9.-", true);
        static std::string_view true0("true");
        static std::string_view false0("false");
        static std::string_view null0("null");

        return !s.empty() && alpha.chars_table[(ui8)s[0]] && alnum.cbrk(s.begin() + 1, s.end()) == s.end() && !EqualToOneOf(s, null0, true0, false0);
    }

    // to keep arcadia tests happy
    bool TJsonPrettifier::MayUnquoteOld(std::string_view s) {
        static str_spn alpha("a-zA-Z_@$", true);
        static str_spn alnum("a-zA-Z_@$0-9", true);
        static std::string_view true0("true");
        static std::string_view false0("false");
        static std::string_view true1("on");
        static std::string_view false1("off");
        static std::string_view true2("da");
        static std::string_view false2("net");
        static std::string_view null0("null");

        return !s.empty() && alpha.chars_table[(ui8)s[0]] && alnum.cbrk(s.begin() + 1, s.end()) == s.end() && !EqualToOneOf(s, null0, true0, false0, true1, false1, true2, false2);
    }

    class TPrettifier: public TJsonCallbacks {
        TRewritableOut Out;
        std::string_view Spaces;
        std::string_view Quote;
        std::string_view Unsafe;
        std::string_view Safe;

        ui32 Level = 0;
        ui32 MaxPaddingLevel;

        bool Unquote = false;
        bool Compactify = false;
        bool NewUnquote = false;

    public:
        TPrettifier(IOutputStream& out, const TJsonPrettifier& p)
            : Out(out)
            , MaxPaddingLevel(p.MaxPaddingLevel)
            , Unquote(p.Unquote)
            , Compactify(p.Compactify)
            , NewUnquote(p.NewUnquote)
        {
            static TSpaces spaces;
            Spaces = spaces.Get(p.Padding);
            if (p.SingleQuotes) {
                Quote = Unsafe = "'";
                Safe = "\"";
            } else {
                Quote = Unsafe = "\"";
                Safe = "'";
            }
        }

        void Pad(bool close = false) {
            if (Compactify) {
                Out.Flush();
                return;
            }
            if (Level > MaxPaddingLevel || (Level == MaxPaddingLevel && close)) {
                Out.Write(" ");
                return;
            }
            if (Level || close) {
                Out.Write(!Spaces.empty() ? "\n" : " ");
            }
            for (ui32 i = 0; i < Level; ++i) {
                Out.Write(Spaces);
            }
        }

        void WriteSpace(char sp) {
            if (Compactify) {
                Out.Flush();
                return;
            }

            Out.Write(sp);
        }

        void OnVal() {
            if (Out.Dirty && ':' == Out.Last) {
                WriteSpace(' ');
            } else {
                Pad();
            }
        }

        void AfterVal() {
            Out.Hold(',');
        }

        template <typename T>
        bool WriteVal(const T& t) {
            OnVal();
            Out.Write(t);
            AfterVal();
            return true;
        }

        bool OnNull() override {
            return WriteVal(std::string_view("null"));
        }

        bool OnBoolean(bool v) override {
            return WriteVal(v ? std::string_view("true") : std::string_view("false"));
        }

        bool OnInteger(long long i) override {
            return WriteVal(i);
        }

        bool OnUInteger(unsigned long long i) override {
            return WriteVal(i);
        }

        bool OnDouble(double d) override {
            return WriteVal(d);
        }

        void WriteString(std::string_view s) {
            if (Unquote && (NewUnquote ? TJsonPrettifier::MayUnquoteNew(s) : TJsonPrettifier::MayUnquoteOld(s))) {
                Out.Slave << s;
            } else {
                Out.Slave << Quote;
                NEscJ::EscapeJ<false, true>(s, Out.Slave, Safe, Unsafe);
                Out.Slave << Quote;
            }
        }

        bool OnString(const std::string_view& s) override {
            OnVal();
            WriteString(s);
            AfterVal();
            return true;
        }

        bool OnOpen(char c) {
            OnVal();
            Level++;
            Out.Hold(c);
            return true;
        }

        bool OnOpenMap() override {
            return OnOpen('{');
        }

        bool OnOpenArray() override {
            return OnOpen('[');
        }

        bool OnMapKey(const std::string_view& k) override {
            OnVal();
            WriteString(k);
            WriteSpace(' ');
            Out.Hold(':');
            return true;
        }

        bool OnClose(char c) {
            if (!Level)
                return false;

            Level--;

            if (Out.Dirty && c == Out.Last) {
                WriteSpace(' ');
            } else {
                Out.Revert();
                Pad(true);
            }

            return true;
        }

        bool OnCloseMap() override {
            if (!OnClose('{'))
                return false;
            Out.Write("}");
            AfterVal();
            return true;
        }

        bool OnCloseArray() override {
            if (!OnClose('['))
                return false;
            Out.Write("]");
            AfterVal();
            return true;
        }

        bool OnEnd() override {
            return !Level;
        }
    };

    bool TJsonPrettifier::Prettify(std::string_view in, IOutputStream& out) const {
        TPrettifier p(out, *this);
        if (Strict) {
            TMemoryInput mIn(in.data(), in.size());
            return ReadJson(&mIn, &p);
        } else {
            return ReadJsonFast(in, &p);
        }
    }

    std::string TJsonPrettifier::Prettify(std::string_view in) const {
        TStringStream s;
        if (Prettify(in, s))
            return s.Str();
        return std::string();
    }

}
