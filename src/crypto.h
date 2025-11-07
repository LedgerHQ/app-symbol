
#pragma once

#include <stdint.h>
#include "os.h"

/**
 * Supported cryptographic curves
 */
typedef enum {
    CURVE_Ed25519 = 1,
    CURVE_256K1 = 2

} CurveType_t;

/**
 * Derive private key given BIP32 path.
 *
 * @param[in]  bip32_path
 *   Pointer to buffer with BIP32 path.
 *   For example the bip32 path m/44'/93'/5'/0/0 would be represented as follows:
 *	 uint32_t bip32Path[] = {44 | 0x80000000, 93 | 0x80000000, 5 | 0x80000000, 0, 0};
 *
 * @param[in]  bip32_path_len
 *   Size of 'bip32_path[]' array
 *
 * @param[in]  curve_type
 *   The curve type
 *
 * @param[out] private_key
 *   The derived private key result.
 *
 */
void crypto_derive_private_key(const uint32_t* bip32_path,
                               const uint8_t bip32_path_len,
                               const CurveType_t curve_type,
                               cx_ecfp_private_key_t* private_key);
