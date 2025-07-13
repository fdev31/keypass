# password_module.pyx
from libc.stdlib cimport free

cdef extern from "_keypass.h":
    void c_set_passphrase(const char *phrase)

    const char* c_dump_single_password(const char *label, const char *password, char layout,
                                  unsigned char version, int index)
    bint c_parse_single_password(const char *rawdata, char *label, char *password, char *layout,
                               unsigned char *version, int index)

def set_passphrase(bytes phrase):
    cdef bytes passphrase = phrase
    c_set_passphrase(passphrase)

def dump_single_password(bytes label, bytes password, char layout,
                     unsigned char version, int index):
    cdef bytes label_bytes = label
    cdef bytes password_bytes = password
    cdef const char* result = c_dump_single_password(label_bytes, password_bytes,
                                                layout, version, index)
    try:
        return result.decode('utf-8')
    finally:
        free(<void*>result)  # Clean up allocated memory

def parse_single_password(bytes rawdata, int index):
    cdef char label[256]  # Adjust size based on MAX_NAME_LEN
    cdef char password[256]  # Adjust size based on MAX_PASS_LEN
    cdef char layout = 0
    cdef unsigned char version = 0

    result = c_parse_single_password(rawdata, label, password, &layout, &version, index)

    if result:
        return {
            'label': label,
            'password': password,
            'layout': layout,
            'version': version
        }
    return None
