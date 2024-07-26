#include <util/generic/string.h>
#include <util/stream/input.h>
#include <util/stream/output.h>
#include <util/generic/deque.h>

int main() {
    TString s;
    TDeque<TString> d;

    Cin >> s;

    s += "abacaba";
    d.push_back(s);

    Cout << s;
    return 0;
}