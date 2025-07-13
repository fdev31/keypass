#ifndef String_h
#define String_h

#include <iostream>
#include <string>

class String {
public:
  String(const char *cstr = "");
  String(const String &other);
  ~String();

  String &operator+=(const String &str);
  String &operator+=(const char *cstr);
  size_t indexOf(char c, size_t pos = 0) const;
  String substring(size_t pos, size_t len = -1) const;
  String &operator=(const String &rhs);
  String &operator=(const char *cstr);
  bool operator==(const String &rhs) const;
  bool operator==(const char *cstr) const;

  void toCharArray(char *buf, unsigned int bufsize,
                   unsigned int index = 0) const;
  const char *c_str() const;
  unsigned int length() const;

  friend std::ostream &operator<<(std::ostream &os, const String &str);

private:
  std::string _str;
};

#endif
