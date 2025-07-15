// password_wrapper.c
#include "crypto.h"
#include "importexport.h"
#include "streamadapter.h"
#include <String.h>
#include <string.h>
#include <time.h>
extern "C" {

const char *c_dump_single_password(const char *label, const char *password,
                                   int layout) {
  StringStreamAdapter stream;

  dumpSinglePassword(stream, label, password, layout, getNonce());
  // Get the C-string representation directly if available
  return strdup(stream.c_str());
}

// Wrapper for parseSinglePassword
bool c_parse_single_password(const char *rawdata, char *label, char *password,
                             int *layout) {
  bool ret = parseSinglePassword(rawdata, label, password, layout);
  // decryptBuffer((const uint8_t *)password, password, index, 32);
  return ret;
}

void c_srandom() { srandom(time(NULL)); }
void c_set_passphrase(const char *phrase) { setPassPhrase(phrase); }
}
