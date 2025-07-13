// password_wrapper.c
#include "crypto.h"
#include "importexport.h"
#include <string.h>
extern "C" {

// C-compatible wrapper for dumpSinglePassword
const char *c_dump_single_password(const char *label, const char *password,
                                   const char layout,
                                   const unsigned char version, int index) {
  String result = dumpSinglePassword(label, password, layout, version, index);
  // Note: This creates a memory leak unless you handle cleanup
  // In a real implementation, you'd need proper memory management
  char *c_str = strdup(result.c_str());
  return c_str;
}

// Wrapper for parseSinglePassword
bool c_parse_single_password(const char *rawdata, char *label, char *password,
                             char *layout, unsigned char *version, int index) {
  return parseSinglePassword(rawdata, label, password, layout, version, index);
}

void c_set_passphrase(const char *phrase) { setPassPhrase(phrase); }
}
