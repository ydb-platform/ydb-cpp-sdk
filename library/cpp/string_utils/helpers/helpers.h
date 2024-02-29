#pragma once

#include <string>

namespace NUtils {

std::string ToLower(const std::string& str);
void ToLower(std::string& str);

std::string ToTitle(const std::string& s);

void RemoveAll(std::string& str, char ch);

bool TrySplitOn(std::string_view src, std::string_view& l, std::string_view& r, size_t pos, size_t len);

bool TrySplit(std::string_view src, std::string_view& l, std::string_view& r, char delim);
bool TrySplit(std::string_view src, std::string_view& l, std::string_view& r, std::string_view delim);
bool TryRSplit(std::string_view src, std::string_view& l, std::string_view& r, char delim);
bool TryRSplit(std::string_view src, std::string_view& l, std::string_view& r, std::string_view delim);

void Split(std::string_view src, std::string_view& l, std::string_view& r, char delim);
void Split(std::string_view src, std::string_view& l, std::string_view& r, std::string_view delim);
void RSplit(std::string_view src, std::string_view& l, std::string_view& r, char delim);
void RSplit(std::string_view src, std::string_view& l, std::string_view& r, std::string_view delim);

std::string_view NextTok(std::string_view& src, char delim);
std::string_view NextTok(std::string_view& src, std::string_view delim);
bool NextTok(std::string_view& src, std::string_view& tok, char delim);
bool NextTok(std::string_view& src, std::string_view& tok, std::string_view delim);

std::string_view RNextTok(std::string_view& src, char delim);
std::string_view RNextTok(std::string_view& src, std::string_view delim);

std::string_view After(std::string_view src, char c);
std::string_view Before(std::string_view src, char c);

}