#pragma once

#include "yql_issue_id.h"

#include <util/system/types.h>
#include <util/generic/hash.h>
#include <util/digest/numeric.h>
#include <google/protobuf/message.h>

#include <vector>
#include <sstream>
#include <string>
#include <string_view>

namespace NYql {

void SanitizeNonAscii(std::string& s);

///////////////////////////////////////////////////////////////////////////////
// TPosition
///////////////////////////////////////////////////////////////////////////////
struct TPosition {
    ui32 Column = 0U;
    ui32 Row = 0U;
    std::string File;

    TPosition() = default;

    TPosition(ui32 column, ui32 row, const std::string& file = {})
        : Column(column)
        , Row(row)
        , File(file)
    {
        SanitizeNonAscii(File);
    }

    explicit operator bool() const {
        return HasValue();
    }

    inline bool HasValue() const {
        return Row | Column;
    }

    inline bool operator==(const TPosition& other) const {
        return Column == other.Column && Row == other.Row && File == other.File;
    }

    inline bool operator<(const TPosition& other) const {
        return std::tie(Row, Column, File) < std::tie(other.Row, other.Column, other.File);
    }
};

class TTextWalker {
public:
    TTextWalker(TPosition& position)
        : Position(position)
        , HaveCr(false)
        , LfCount(0)
    {
    }

    template<typename T>
    TTextWalker& Advance(const T& buf) {
        for (char c : buf) {
            Advance(c);
        }
        return *this;
    }

    TTextWalker& Advance(char c);

private:
    TPosition& Position;
    bool HaveCr;
    ui32 LfCount;
};

struct TRange {
    TPosition Position;
    TPosition EndPosition;

    TRange() = default;

    TRange(TPosition position)
        : Position(position)
        , EndPosition(position)
    {
    }

    TRange(TPosition position, TPosition endPosition)
        : Position(position)
        , EndPosition(endPosition)
    {
    }

    inline bool IsRange() const {
        return !(Position == EndPosition);
    }
};

///////////////////////////////////////////////////////////////////////////////
// TIssue
///////////////////////////////////////////////////////////////////////////////

class TIssue;
using TIssuePtr = TIntrusivePtr<TIssue>;
class TIssue: public TThrRefBase {
    std::vector<TIntrusivePtr<TIssue>> Children_;
    std::string Message;
public:
    TPosition Position;
    TPosition EndPosition;
    TIssueCode IssueCode = 0U;
    ESeverity Severity = TSeverityIds::S_ERROR;

    TIssue() = default;

    template <typename T>
    explicit TIssue(const T& message)
        : Message(message)
        , Position(TPosition())
        , EndPosition(TPosition())
    {
        SanitizeNonAscii(Message);
    }

    template <typename T>
    TIssue(TPosition position, const T& message)
        : Message(message)
        , Position(position)
        , EndPosition(position)
    {
        SanitizeNonAscii(Message);
    }

    inline TRange Range() const {
        return {Position, EndPosition};
    }

    template <typename T>
    TIssue(TPosition position, TPosition endPosition, const T& message)
        : Message(message)
        , Position(position)
        , EndPosition(endPosition)
    {
        SanitizeNonAscii(Message);
    }

    inline bool operator==(const TIssue& other) const {
        return Position == other.Position && Message == other.Message
            && IssueCode == other.IssueCode;
    }

    ui64 Hash() const noexcept {
        return CombineHashes(
            CombineHashes(
                (size_t)CombineHashes(IntHash(Position.Row), IntHash(Position.Column)),
                ComputeHash(Position.File)
            ),
            (size_t)CombineHashes((size_t)IntHash(static_cast<int>(IssueCode)), ComputeHash(Message)));
    }

    TIssue& SetCode(TIssueCode id, ESeverity severity) {
        IssueCode = id;
        Severity = severity;
        return *this;
    }

    TIssue& SetMessage(const std::string& msg) {
        Message = msg;
        SanitizeNonAscii(Message);
        return *this;
    }

    ESeverity GetSeverity() const {
        return Severity;
    }

    TIssueCode GetCode() const {
        return IssueCode;
    }

    const std::string& GetMessage() const {
        return Message;
    }

    TIssue& AddSubIssue(TIntrusivePtr<TIssue> issue) {
        Severity = (ESeverity)Min((ui32)issue->GetSeverity(), (ui32)Severity);
        Children_.push_back(issue);
        return *this;
    }

    const std::vector<TIntrusivePtr<TIssue>>& GetSubIssues() const {
        return Children_;
    }

    void PrintTo(std::ostream& out, bool oneLine = false) const;

    std::string ToString(bool oneLine = false) const {
        std::stringstream out;
        PrintTo(out, oneLine);
        return out.str();
    }

    // Unsafe method. Doesn't call SanitizeNonAscii(Message)
    std::string* MutableMessage() {
        return &Message;
    }

    TIssue& CopyWithoutSubIssues(const TIssue& src) {
        Message = src.Message;
        IssueCode = src.IssueCode;
        Severity = src.Severity;
        Position = src.Position;
        EndPosition = src.EndPosition;
        return *this;
    }
};

void WalkThroughIssues(const TIssue& topIssue, bool leafOnly, std::function<void(const TIssue&, ui16 level)> fn, std::function<void(const TIssue&, ui16 level)> afterChildrenFn = {});

///////////////////////////////////////////////////////////////////////////////
// TIssues
///////////////////////////////////////////////////////////////////////////////
class TIssues {
public:
    TIssues() = default;

    inline TIssues(const std::vector<TIssue>& issues)
        : Issues_(issues)
    {
    }

    inline TIssues(const std::initializer_list<TIssue>& issues)
        : TIssues(std::vector<TIssue>(issues))
    {
    }

    inline TIssues(const TIssues& rhs)
        : Issues_(rhs.Issues_)
    {
    }

    inline TIssues& operator=(const TIssues& rhs) {
        Issues_ = rhs.Issues_;
        return *this;
    }

    inline TIssues(TIssues&& rhs) : Issues_(std::move(rhs.Issues_))
    {
    }

    inline TIssues& operator=(TIssues&& rhs) {
        Issues_ = std::move(rhs.Issues_);
        return *this;
    }

    template <typename ... Args> void AddIssue(Args&& ... args) {
        Issues_.emplace_back(std::forward<Args>(args)...);
    }

    inline void AddIssues(const TIssues& errors) {
        Issues_.insert(Issues_.end(),
                       errors.Issues_.begin(), errors.Issues_.end());
    }

    inline void AddIssues(const TPosition& pos, const TIssues& errors) {
        Issues_.reserve(Issues_.size() + errors.Size());
        for (const auto& e: errors) {
            TIssue& issue = Issues_.emplace_back();
            *issue.MutableMessage() = e.GetMessage(); // No need to sanitize message, it has already been sanitized.
            issue.Position = pos;
            issue.SetCode(e.IssueCode, e.Severity);
        }
    }

    inline const TIssue* begin() const {
        return Issues_.data();
    }

    inline const TIssue* end() const {
        return Issues_.data() + Issues_.size();
    }

    inline TIssue& back() {
        return Issues_.back();
    }

    inline const TIssue& back() const {
        return Issues_.back();
    }

    inline bool Empty() const {
        return Issues_.empty();
    }

    explicit operator bool() const noexcept {
        return !Issues_.empty();
    }

    inline size_t Size() const {
        return Issues_.size();
    }

    void PrintTo(std::ostream& out, bool oneLine = false) const;
    void PrintWithProgramTo(
            std::ostream& out,
            const std::string& programFilename,
            const std::string& programText) const;

    inline std::string ToString(bool oneLine = false) const {
        std::stringstream out;
        PrintTo(out, oneLine);
        return out.str();
    }

    std::string ToOneLineString() const {
        return ToString(true);
    }

    inline void Clear() {
        Issues_.clear();
    }

    void Reserve(size_t capacity) {
        Issues_.reserve(capacity);
    }

private:
    std::vector<TIssue> Issues_;
};

class TErrorException : public yexception {
    const TIssueCode Code_;
public:
    explicit TErrorException(TIssueCode code)
        : Code_(code)
    {}
    TIssueCode GetCode() const {
        return Code_;
    }
};

TIssue ExceptionToIssue(const std::exception& e, const TPosition& pos = TPosition());
std::optional<TPosition> TryParseTerminationMessage(std::string_view& message);

} // namespace NYql

std::ostream& operator<<(std::ostream& out, const NYql::TPosition& pos);
std::ostream& operator<<(std::ostream& out, const NYql::TRange& range);
std::ostream& operator<<(std::ostream& out, const NYql::TIssue& error);
std::ostream& operator<<(std::ostream& out, const NYql::TIssues& error);

template <>
struct THash<NYql::TIssue> {
    inline size_t operator()(const NYql::TIssue& err) const {
        return err.Hash();
    }
};
