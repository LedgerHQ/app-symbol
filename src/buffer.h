/*****************************************************************************
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

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "limitations.h"

/**
 * Struct for buffer with size and offset.
 */
typedef struct {
    const uint8_t* ptr;  /// Pointer to byte buffer
    size_t size;         /// Size of 'ptr' array
    size_t offset;       /// Offset in 'ptr' array
} buffer_t;

/**
 * Tell whether buffer can read bytes or not.
 *
 * @param[in] buffer
 *   Pointer to input buffer struct.
 * @param[in] n
 *   Number of bytes to read in buffer.
 *
 * @return true if success, false otherwise.
 *
 */
bool buffer_can_read(const buffer_t* buffer, size_t n);

/**
 * Seek buffer relatively to current offset.
 *
 * @param[in,out] buffer
 *   Pointer to input buffer struct.
 * @param[in]     offset
 *   Offset to seek relatively to `buffer->offset`.
 *
 * @return true if success, false otherwise.
 *
 */
bool buffer_seek(buffer_t* buffer, size_t offset);

/**
 * Returns pointer to current offset within buffer
 *
 * @param[in] buffer
 *   Pointer to input buffer struct.
 *
 * @return pointer to current offset within buffer
 */
const uint8_t* buffer_offset_ptr(buffer_t* buffer);

/**
 * Returns pointer to current offset within buffer, and then seek buffer relatively to offset.
 *
 * @param[in,out] buffer
 *   Pointer to input buffer struct.
 *
 * @param[in] offset
 *   Offset to seek relatively to `buffer->offset`
 *
 * @return pointer to current offset within buffer
 */
const uint8_t* buffer_offset_ptr_and_seek(buffer_t* buffer, size_t n);

/**
 *  Derive bip32 path from a raw APDU byte buffer
 *
 * @param[in] buffer
 *   The raw APDU byte buffer from host with the bip32path
 *
 * @param[out] bip32Path
 *   Converted bip32Path. bip32Path[n] indicates the child path at level n.
 *   Numbers 0 to (2^31-1) are used for unhardened keys and numbers
 *   (2^31) to (2^32-1) for hardened keys.
 *
 * @return The length of the bip32 path or '0' if there is an error.
 */
uint8_t buffer_get_bip32_path(const buffer_t* buffer, uint32_t bip32Path[MAX_BIP32_PATH]);
