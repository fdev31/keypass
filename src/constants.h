// constants.h
#ifndef _CONSTANTS_H
#define _CONSTANTS_H

// TODO: measure the maximum acceptable number more accurately
#define MAX_PASSWORDS 100
#define STORED_PASSWD_BLOCKS 2

// Crypto constants
#define BLOCK_SIZE 16
#define RAND_BYTE_FACTOR RAND_MAX / 255

#define MAX_NAME_LEN 30
#define MAX_PASS_LEN (16 * STORED_PASSWD_BLOCKS)

// Preferences fields
#define F_NAMESPACE "KeyPass" // used for global settings
// password entry properties
#define F_NAME "name"
#define F_PASSWORD "password"
#define F_FORMAT "v"
#define F_LAYOUT "layout"

#endif // CONSTANTS_H
