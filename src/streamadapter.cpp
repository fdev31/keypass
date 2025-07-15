#include "Arduino.h"
#ifdef ESP32
#include <StreamString.h>
#else
#include <sstream>
#endif

class StringStreamAdapter {
private:
#ifdef ESP32
  StreamString *stream;
  bool ownsStream;
#else
  std::stringstream *stream;
  bool ownsStream;
#endif

public:
  // Clear content
  void clear() {
#ifdef ESP32
    stream->clear(); // If available, otherwise you may need to create a new
                     // stream
#else
    stream->str("");
    stream->clear();
#endif
  }

  // Add more common methods as needed...
};
