#pragma once
#include <stdint.h>

typedef enum {
    OK = 0x9000,
    NO_APDU_RECEIVED = 0x6982,
    UNKNOWN_INSTRUCTION_CLASS = 0x6E00,
    UNKNOWN_INSTRUCTION = 0x6D00,
    WRONG_APDU_DATA_LENGTH = 0x6A87,

    INVALID_PKG_KEY_LENGTH = 0x6A80,
    INVALID_BIP32_PATH_LENGTH = 0x6A81,
    INVALID_P1_OR_P2 = 0x6B00,
    WRONG_RESPONSE_LENGTH = 0xB000,

    ADDRESS_REJECTED = 0x6985,
    TRANSACTION_REJECTED = 0x6986,

    INVALID_SIGNING_PACKET_ORDER = 0x6A82,
    SIGNING_DATA_TOO_LARGE = 0x6700,
    TOO_MANY_TRANSACTION_FIELDS = 0x6701,
    INVALID_TRANSACTION_DATA = 0x6702,
    INVALID_INTERNAL_SIGNING_STATE = 0x6703,
    INVALID_SIGNING_DATA = 0x6A82,

    INTERNAL_ERROR = 0x6A83
} ApduResponse_t;

/**
 * Enumeration with expected INS of APDU commands.
 */
typedef enum {
    GET_PUBLIC_KEY = 0x02,  /// public key of corresponding BIP32 path
    SIGN_TX = 0x04,         /// sign transaction with BIP32 path
    GET_VERSION = 0x06,     /// version of the application
} ApduInstruction_t;

/**
 * Structure with fields of APDU command.
 */
typedef struct {
    uint8_t cla;            /// Instruction class
    ApduInstruction_t ins;  /// Instruction code
    uint8_t p1;             /// Instruction parameter 1
    uint8_t p2;             /// Instruction parameter 2
    uint8_t lc;             /// Length of command data
    uint8_t* data;          /// Command data
} ApduCommand_t;
