// password_wrapper.h
#ifndef PASSWORD_WRAPPER_H
#define PASSWORD_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

void c_srandom();
void c_set_passphrase(const char *phrase);

const char *c_dump_single_password(const char *label, const char *password,
                                   char layout);

bool c_parse_single_password(const char *rawdata, char *label, char *password,
                             char *layout);

#ifdef __cplusplus
}
#endif

#endif // PASSWORD_WRAPPER_H
