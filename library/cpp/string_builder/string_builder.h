#pragma once

#include <string>
#include <sstream>

namespace NUtils {

class TYdbStringBuilder {
    using TStdStreamManipulator = std::ostream& (*)(std::ostream&);
public:
    template <class T>
    TYdbStringBuilder& operator<<(const T& value) {
        stream << value;
        return *this;
    }

    TYdbStringBuilder& operator<<(TStdStreamManipulator mod) {
        mod(stream);
        return *this;
    }

    operator std::string() {
        return stream.str();
    }
private:
    std::stringstream stream;
};

}

