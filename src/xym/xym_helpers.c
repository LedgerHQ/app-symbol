/*******************************************************************************
 *   XYM Wallet
 *   (c) 2020 FDS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ********************************************************************************/
#include <string.h>
#include "ledger_assert.h"
#include "base32.h"
#include "xym_helpers.h"

#ifdef FUZZ
#include <bsd/string.h>
#endif  // FUZZ

void xym_print_amount(uint64_t amount,
                      uint8_t divisibility,
                      const char *asset,
                      char *out,
                      size_t outlen) {
    char buffer[AMOUNT_MAX_SIZE];
    uint64_t dVal = amount;
    int i, j;
    uint8_t MAX_DIVISIBILITY = (divisibility == 0) ? 0 : 6;

    memset(buffer, 0, AMOUNT_MAX_SIZE);
    for (i = 0; dVal > 0 || i < MAX_DIVISIBILITY + 1; i++) {
        if (dVal > 0) {
            buffer[i] = (dVal % 10) + '0';
            dVal /= 10;
        } else {
            buffer[i] = '0';
        }
        if (i == divisibility - 1) {  // divisibility
            i += 1;
            buffer[i] = '.';
            if (dVal == 0) {
                i += 1;
                buffer[i] = '0';
            }
        }

        LEDGER_ASSERT(i < AMOUNT_MAX_SIZE - 1, "Amount is too large to be printed");
    }
    // reverse order
    for (i -= 1, j = 0; i >= 0 && j < AMOUNT_MAX_SIZE - 1; i--, j++) {
        out[j] = buffer[i];
    }
    // strip trailing 0s
    if (MAX_DIVISIBILITY != 0) {
        for (j -= 1; j > 0; j--) {
            if (out[j] != '0') break;
        }
        j += 1;
    }

    // strip trailing .
    if (out[j - 1] == '.') j -= 1;

    if (asset && strlen(asset) > 0) {
        out[j++] = ' ';
        strlcpy(out + j, asset, outlen - j);
        out[j + strlen(asset)] = '\0';
    } else {
        out[j] = '\0';
    }
}

#ifndef FUZZ
int sha_calculation(uint8_t *in, uint8_t inlen, uint8_t *out, uint8_t outlen) {
    int error = SWO_PARAMETER_ERROR_NO_INFO;
    cx_sha3_t hash;
    CX_CHECK(cx_sha3_init_no_throw(&hash, 256));
    CX_CHECK(cx_hash_no_throw(&hash.header, CX_LAST, in, inlen, out, outlen));
end:
    return error;
}

int ripemd(uint8_t *in, uint8_t inlen, uint8_t *out, uint8_t outlen) {
    cx_ripemd160_t hash;
    cx_ripemd160_init(&hash);
    return cx_hash_no_throw(&hash.header, CX_LAST, in, inlen, out, outlen);
}

int xym_public_key_and_address(cx_ecfp_public_key_t *inPublicKey,
                               uint8_t inNetworkId,
                               uint8_t *outPublicKey,
                               char *outAddress,
                               uint8_t outLen) {
    // TODO: use defines instead hardcoded numbers
    uint8_t buffer1[32];
    uint8_t buffer2[20];
    uint8_t rawAddress[32];
    int error = SWO_PARAMETER_ERROR_NO_INFO;

    for (uint8_t i = 0; i < 32; i++) {
        outPublicKey[i] = inPublicKey->W[64 - i];
    }
    if ((inPublicKey->W[32] & 1) != 0) {
        outPublicKey[31] |= 0x80;
    }
    CX_CHECK(sha_calculation(outPublicKey, 32, buffer1, sizeof(buffer1)));
    CX_CHECK(ripemd(buffer1, 32, buffer2, sizeof(buffer2)));
    // step1: add network prefix char
    rawAddress[0] = inNetworkId;
    // step2: add ripemd160 hash
    memcpy(rawAddress + 1, buffer2, sizeof(buffer2));
    CX_CHECK(sha_calculation(rawAddress, 21, buffer1, sizeof(buffer1)));
    // step3: add checksum
    memcpy(rawAddress + 21, buffer1, 3);
    rawAddress[24] = 0;
    if (base32_encode((const uint8_t *) rawAddress, 24, (char *) outAddress, outLen) < 0) {
        error = SWO_DATA_MAY_BE_CORRUPTED;
        goto end;
    }
    error = SWO_SUCCESS;
end:
    return error;
}
#endif
