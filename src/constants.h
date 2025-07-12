// constants.h
#ifndef _CONSTANTS_H
#define _CONSTANTS_H

// TODO: measure the maximum acceptable number more accurately
#define MAX_PASSWORDS 100
#define STORED_PASSWD_BLOCKS 2

// Crypto constants
#define STO_BLOCK_SIZE 16
#define RAND_BYTE_FACTOR RAND_MAX / 255

#define MAX_NAME_LEN 32 // Must be a multiple of block size
#define MAX_PASS_LEN (STORED_PASSWD_BLOCKS * 16)

// ChaCha: 12 nonce bytes + 4 bytes length
// #define CRYPTO_OVERHEAD 16
// Speck: zero
#define CRYPTO_OVERHEAD 0

#define DUMP_LENGTH (MAX_PASS_LEN + MAX_NAME_LEN + 2 + CRYPTO_OVERHEAD)

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
