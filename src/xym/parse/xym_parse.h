/*******************************************************************************
 *    XYM Wallet
 *    (c) 2020 FDS
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
#ifndef LEDGER_APP_XYM_XYMPARSE_H
#define LEDGER_APP_XYM_XYMPARSE_H

#include "limitations.h"
#include "fields.h"
#include "xym_helpers.h"
#include "app_buffer.h"

#define XYM_PERSISTENT_DELEGATED_HARVESTING 0xFE
#define ALIGNMENT_BYTES                     8

#pragma pack(push, 1)
typedef struct {
    uint64_t mosaicId;
    uint64_t amount;
} mosaic_t;

typedef struct {
    uint64_t maxFee;
    uint64_t deadline;
} txn_fee_t;

#pragma pack(pop)

typedef struct {
    uint8_t numFields;
    field_t arr[MAX_FIELD_COUNT];
} fields_array_t;

/**
 * Given a buffer with a transaction serialization, parses the buffer and
 * extracts parameters and creates a fields array to be displayed to the
 * user for verification.
 *
 * The symbol serializations are defined here:
 * https://docs.symbolplatform.com/serialization/index.html
 *
 * @param[in]  rawTxdata  A buffer with the raw tx serialized data
 * @param[out] fields     An array with the individual transaction fields
 * @return                one of the codes in the '_parser_error' enum
 */
int parse_txn_context(buffer_t* rawTxdata, fields_array_t* fields);

#endif  // LEDGER_APP_XYM_XYMPARSE_H
