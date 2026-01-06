#pragma once
#include <stdint.h>
#include "status_words.h"

#define TOO_MANY_TRANSACTION_FIELDS    0x6701
#define INVALID_TRANSACTION_DATA       0x6702
#define INVALID_INTERNAL_SIGNING_STATE 0x6703
#define INVALID_BIP32_PATH_LENGTH      0x6A81
#define INVALID_SIGNING_PACKET_ORDER   0x6A82
#define INVALID_SIGNING_DATA           0x6A82
#define INTERNAL_ERROR                 0x6A83

/**
 * Enumeration with expected INS of APDU commands.
 */
typedef enum {
    GET_PUBLIC_KEY = 0x02,  /// public key of corresponding BIP32 path
    SIGN_TX = 0x04,         /// sign transaction with BIP32 path
    GET_VERSION = 0x06,     /// version of the application
} ApduInstruction_t;
