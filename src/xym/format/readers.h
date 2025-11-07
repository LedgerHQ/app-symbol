/*******************************************************************************
 *   XYM Wallet
 *   (c) 2017 Ledger
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
#ifndef LEDGER_APP_XYM_READERS_H
#define LEDGER_APP_XYM_READERS_H

#include <stdint.h>

static inline int8_t read_int8(const uint8_t *src) {
    return *(const int8_t *) src;
}

static inline int16_t read_int16(const uint8_t *src) {
    return (int16_t) src[0] << 0 | (int16_t) src[1] << 8;
}

static inline uint8_t read_uint8(const uint8_t *src) {
    return *(const uint8_t *) src;
}

static inline uint16_t read_uint16(const uint8_t *src) {
    return (uint16_t) src[0] << 0 | (uint16_t) src[1] << 8;
}

static inline uint16_t read_uint32(const uint8_t *src) {
    return (uint64_t) src[0] << 0 | (uint64_t) src[1] << 8 | (uint64_t) src[2] << 16 |
           (uint64_t) src[3] << 24;
}

static inline uint64_t read_uint64(const uint8_t *src) {
    return (uint64_t) src[0] << 0 | (uint64_t) src[1] << 8 | (uint64_t) src[2] << 16 |
           (uint64_t) src[3] << 24 | (uint64_t) src[4] << 32 | (uint64_t) src[5] << 40 |
           (uint64_t) src[6] << 48 | (uint64_t) src[7] << 56;
}
#endif  // LEDGER_APP_XYM_READERS_H
