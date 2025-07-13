// constants.h
#ifndef _CONSTANTS_H
#define _CONSTANTS_H

// TODO: measure the maximum acceptable number more accurately
#define MAX_PASSWORDS 100
#define STORED_PASSWD_BLOCKS 2

// Crypto constants
#define STO_BLOCK_SIZE 16
#define RAND_BYTE_FACTOR RAND_MAX / 255

#define CRYPTO_HEADER_SIZE 2 // salt + layout

#define META_SIZE 32
#define MAX_NAME_LEN META_SIZE - CRYPTO_HEADER_SIZE
#define MAX_PASS_LEN (STORED_PASSWD_BLOCKS * 16)

#define CRYPTO_OVERHEAD 0
#define UNENCRYPTED_DATA_LENGTH 1 // version

// Encrypted dump length
#define DUMP_LENGTH (META_SIZE + MAX_PASS_LEN + CRYPTO_OVERHEAD)
static_assert(DUMP_LENGTH % STO_BLOCK_SIZE == 0,
              "DUMP_LENGTH must be a multiple of STO_BLOCK_SIZE");

// Preferences fields
#define F_NAMESPACE "KeyPass" // used for global settings
// password entry properties
#define F_NAME "name"
#define F_PASSWORD "password"
#define F_FORMAT "v"
#define F_LAYOUT "layout"

#define DUMP_START "#KPDUMP"
#define DUMP_END "#/KPDUMP"

#endif // CONSTANTS_H
