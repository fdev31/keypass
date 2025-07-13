#include "String.h"
#include <string.h>

String::String(const char *cstr) : _str(cstr) {}
String::String(const String &other) : _str(other._str) {}
String::~String() {}

String & String::operator += (const String &str) {
    _str += str._str;
    return *this;
}

String & String::operator += (const char *cstr) {
    _str += cstr;
    return *this;
}

size_t String::find(char c, size_t pos) const {
    return _str.find(c, pos);
}

String String::substr(size_t pos, size_t len) const {
    return String(_str.substr(pos, len).c_str());
}

String & String::operator = (const String &rhs) {
    _str = rhs._str;
    return *this;
}

String & String::operator = (const char *cstr) {
    _str = cstr;
    return *this;
}

bool String::operator == (const String &rhs) const {
    return _str == rhs._str;
}

bool String::operator == (const char *cstr) const {
    return _str == cstr;
}

void String::toCharArray(char *buf, unsigned int bufsize, unsigned int index) const {
    strncpy(buf, _str.c_str() + index, bufsize);
}

const char* String::c_str() const {
    return _str.c_str();
}

unsigned int String::length() const {
    return _str.length();
}

std::ostream& operator<<(std::ostream& os, const String& str) {
    os << str._str;
    return os;
}
