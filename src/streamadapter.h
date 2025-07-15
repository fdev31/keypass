// StringStreamAdapter.h
#ifndef STRING_STREAM_ADAPTER_H
#define STRING_STREAM_ADAPTER_H

#include <Arduino.h>
#ifdef ESP32
#include <StreamString.h>
#else
#include <sstream>
#include <iomanip>
#endif

class StringStreamAdapter {
private:
#ifdef ESP32
  StreamString *stream;
#else
  std::stringstream *stream;
  std::string _str;
#endif
  bool ownsStream;

public:
  // Constructors and destructor
  inline StringStreamAdapter(){
#ifdef ESP32
    stream = new StreamString();
#else
    stream = new std::stringstream();
#endif
  }

#ifdef ESP32
  inline StringStreamAdapter(StreamString *existingStream)
      : stream(existingStream), ownsStream(false) {}
#else
  inline StringStreamAdapter(std::stringstream *existingStream)
      : stream(existingStream), ownsStream(false) {}
#endif

  inline ~StringStreamAdapter(){
    if (ownsStream && stream) {
      delete stream;
    }
  }

  // Prevent copying
  StringStreamAdapter(const StringStreamAdapter &) = delete;
  StringStreamAdapter &operator=(const StringStreamAdapter &) = delete;

  // Common operations
  inline StringStreamAdapter &write(const char *str){
#ifdef ESP32
    stream->print(str);
#else
    (*stream) << str;
#endif
    return *this;
  }
  inline StringStreamAdapter &write(int value){
#ifdef ESP32
    stream->print(value);
#else
    (*stream) << value;
#endif
    return *this;
  }
  inline StringStreamAdapter &write(float value, int decimals = 2){
    // ESP32's print for float handles decimals
#ifdef ESP32
    stream->print(value, decimals);
#else
    // For std::stringstream, we need to manipulate stream flags
    (*stream) << std::fixed << std::setprecision(decimals) << value;
#endif
    return *this;
  }
  inline StringStreamAdapter &write(double value, int decimals = 2){
#ifdef ESP32
    stream->print(value, decimals);
#else
    (*stream) << std::fixed << std::setprecision(decimals) << value;
#endif
    return *this;
  }
  inline StringStreamAdapter &writeLine(const char *str){
    write(str);
#ifdef ESP32
    stream->println();
#else
    (*stream) << std::endl;
#endif
    return *this;
  }

  // Get content
  inline String toString() const {
#ifdef ESP32
    return String(stream->c_str());
#else
    return stream->str().c_str();
#endif
  }
  
  inline const char *c_str() {
#ifdef ESP32
    return stream->c_str();
#else
    _str = stream->str();
    return _str.c_str();
#endif
  }

  // Get raw stream pointer (when needed for direct access)
#ifdef ESP32
  StreamString *getStream() { return stream; }
#else
  std::stringstream *getStream() { return stream; }
#endif

  // Stream manipulation
  inline void clear() {
#ifdef ESP32
    // This relies on StreamString having a clear() method,
    // or you might need to re-initialize the stream if you own it.
    stream->clear();
#else
    stream->str("");
    stream->clear(); // Clear error flags
#endif
  }

  inline size_t length() const {
#ifdef ESP32
    return stream->length();
#else
    return stream->str().length();
#endif
  }

  inline bool isEmpty() const {
#ifdef ESP32
    return stream->length() == 0;
#else
    return stream->str().empty();
#endif
  }

  // Operator overloads for convenient usage
  template <typename T> StringStreamAdapter &operator<<(const T &value) {
    write(value);
    return *this;
  }

  operator String() const { return toString(); }
};

#endif // STRING_STREAM_ADAPTER_H
