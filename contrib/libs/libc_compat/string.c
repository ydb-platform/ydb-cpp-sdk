#include <ctype.h>
#include <string.h>

char* strlwr(char* s) {
    char* d;
    for (d = s; *d; ++d)
        *d = (char)tolower((int)*d);
    return s;
}
