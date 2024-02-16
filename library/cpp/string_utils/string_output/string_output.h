#include <util/stream/output.h>

#include <string>

namespace NUtils {

class TStringOutput: public IOutputStream {
public:
    inline TStringOutput(std::string& s) noexcept
        : S_(&s)
    {
    }

    inline void Reserve(size_t size);
    inline void Undo(size_t len);

private:
    void DoWrite(const void* buf, size_t len) override;
    void DoWriteC(char c) override;

    std::string* S_;
};

}