#include <library/cpp/resource/registry.h>

#include <util/stream/output.h>
#include <util/stream/file.h>
#include <util/digest/city.h>
#include <util/string/cast.h>
#include <util/string/hex.h>
#include <util/string/vector.h>
#include <util/string/split.h>

using namespace NResource;

static inline void GenOne(const std::string& data, const std::string& key, IOutputStream& out) {
    const std::string name = "name" + ToString(CityHash64(key.data(), key.size()));

    out << "static const unsigned char " << name << "[] = {";

    const std::string c = Compress(data);
    char buf[16];

    for (size_t i = 0; i < c.size(); ++i) {
        if ((i % 10) == 0) {
            out << "\n    ";
        }

        const char ch = c[i];

        out << "0x" << std::string_view(buf, HexEncode(&ch, 1, buf)) << ", ";
    }

    out << "\n};\n\nstatic const NResource::TRegHelper REG_" << name << "(\"" << key << "\", std::string_view((const char*)" << name << ", sizeof(" << name << ")));\n";
}

int main(int argc, char** argv) {
    if ((argc < 4) || (argc % 2)) {
        Cerr << "usage: " << argv[0] << " outfile [infile path]+ [- key=value]+" << Endl;

        return 1;
    }

    TFixedBufferFileOutput out(argv[1]);

    argv = argv + 2;

    out << "#include <library/cpp/resource/registry.h>\n\n";

    while (*argv) {
        if ("-"sv == *argv) {
            std::vector<std::string> items = StringSplitter(std::string(*(argv + 1))).Split('=').Limit(2).ToList<std::string>();
            GenOne(items[1], items[0], out);
        } else {
            const char* key = *(argv + 1);
            if (*key == '-') {
                ++key;
            }
            GenOne(TUnbufferedFileInput(*argv).ReadAll(), key, out);
        }
        argv += 2;
    }
}
