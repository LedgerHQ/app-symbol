/*****************************************************************************
 *   Ledger App Boilerplate.
 *   (c) 2020 Ledger SAS.
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
 *****************************************************************************/
#include <string.h>
#include "crypto.h"
#include "xym_helpers.h"
#include "limitations.h"

int crypto_derive_private_key(const uint32_t* bip32_path,
                              const uint8_t bip32_path_len,
                              const CurveType_t curve_type,
                              cx_ecfp_private_key_t* private_key) {
    int error = SWO_PARAMETER_ERROR_NO_INFO;
    uint8_t raw_private_key[XYM_RAW_PRIVATE_KEY_LENGTH] = {0};

    // derive the seed with bip32_path
    if (curve_type == CURVE_Ed25519) {
        unsigned char seed_key[] = "ed25519 seed";
        CX_CHECK(os_derive_bip32_with_seed_no_throw(HDW_ED25519_SLIP10,
                                                    CX_CURVE_Ed25519,
                                                    bip32_path,
                                                    bip32_path_len,
                                                    raw_private_key,
                                                    NULL,
                                                    seed_key,
                                                    sizeof(seed_key) - 1));

    } else {
        CX_CHECK(os_derive_bip32_no_throw(CX_CURVE_256K1,
                                          bip32_path,
                                          bip32_path_len,
                                          raw_private_key,
                                          NULL));
    }

    // initialize private_key from raw key
    CX_CHECK(cx_ecfp_init_private_key_no_throw(CX_CURVE_Ed25519,
                                               raw_private_key,
                                               XYM_PRIVATE_KEY_LENGTH,
                                               private_key));
    error = SWO_SUCCESS;
end:
    explicit_bzero(&raw_private_key, sizeof(raw_private_key));
    return error;
}
