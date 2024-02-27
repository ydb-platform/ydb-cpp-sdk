#pragma once

#include <util/generic/string.h>

// Some functions for inverted url representation
// No scheme cut-off, no 80th port normalization

void TrspChars(char* s);
void UnTrspChars(char* s);
void TrspChars(char* s, size_t l);
void UnTrspChars(char* s, size_t l);
void TrspChars(const char* s, char* d);
void UnTrspChars(const char* s, char* d);

void InvertDomain(char* begin, char* end);

inline std::string& InvertDomain(std::string& url) {
    InvertDomain(url.begin(), url.begin() + url.size());
    return url;
}

void InvertUrl(char* begin, char* end);

inline void InvertUrl(char* url) {
    InvertUrl(url, url + strlen(url));
}

inline std::string& InvertUrl(std::string& url) {
    InvertUrl(url.begin(), url.begin() + url.size());
    return url;
}

void RevertUrl(char* begin, char* end);

inline void RevertUrl(char* url) {
    RevertUrl(url, url + strlen(url));
}

inline std::string& RevertUrl(std::string& url) {
    RevertUrl(url.begin(), url.begin() + url.size());
    return url;
}
