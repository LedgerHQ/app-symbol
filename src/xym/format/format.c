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
#include <string.h>
#include <inttypes.h>
#include "app_format.h"
#include "fields.h"
#include "readers.h"
#include "printers.h"
#include "global.h"
#include "xym_helpers.h"
#include "common.h"
#include "base32.h"

typedef void (*field_formatter_t)(const field_t *field, char *dst);

static void int8_formatter(const field_t *field, char *dst) {
    int8_t value = read_int8(field->data);
    if (value > 0) {
        SNPRINTF(dst, "%s %d %s", "Add", value, "address(es)");
    } else if (value < 0) {
        SNPRINTF(dst, "%s %d %s", "Remove", ~value + 1, "address(es)");
    } else {
        SNPRINTF(dst, "%s", "Not change");
    }
}

static void int16_formatter(const field_t *field, char *dst) {
    int16_t value = read_int16(field->data);
    if (value > 0) {
        SNPRINTF(dst, "%s %d %s", "Increase", value, "byte(s)");
    } else if (value < 0) {
        SNPRINTF(dst, "%s %d %s", "Decrease", ~value + 1, "byte(s)");
    } else {
        SNPRINTF(dst, "%s", "Not change");
    }
}

static void uint8_formatter(const field_t *field, char *dst) {
    uint8_t value = read_uint8(field->data);
    if (field->id == XYM_UINT8_MOSAIC_COUNT) {
        SNPRINTF(dst, "Found %d", value);
    } else if (field->id == XYM_UINT8_TXN_MESSAGE_TYPE) {
        if (value == 0x00) {
            SNPRINTF(dst, "%s", "Plain text");
        } else if (value == 0x01) {
            SNPRINTF(dst, "%s", "Encrypted text");
        } else if (value == 0xFE) {
            SNPRINTF(dst, "%s", "Persistent harvesting delegation");
        }
    } else if (field->id == XYM_UINT8_AA_TYPE) {
        if (value == 0) {
            SNPRINTF(dst, "%s", "Unlink address");
        } else if (value == 1) {
            SNPRINTF(dst, "%s", "Link address");
        }
    } else if (field->id == XYM_UINT8_KL_TYPE) {
        if (value == 0x00) {
            SNPRINTF(dst, "%s", "Unlink");
        } else if (value == 0x01) {
            SNPRINTF(dst, "%s", "Link");
        }
    } else if (field->id == XYM_UINT8_NS_REG_TYPE) {
        if (value == 0) {
            SNPRINTF(dst, "%s", "Root namespace");
        } else if (value == 1) {
            SNPRINTF(dst, "%s", "Sub namespace");
        }
    } else if (field->id == XYM_UINT8_MSC_ACTION) {
        if (value == 0) {
            SNPRINTF(dst, "%s", "Decrease");
        } else if (value == 1) {
            SNPRINTF(dst, "%s", "Increase");
        }
    } else if (field->id == XYM_UINT8_MD_SUPPLY_FLAG) {
        if ((value & 0x01) != 0) {
            SNPRINTF(dst, "%s", "Yes");
        } else {
            SNPRINTF(dst, "%s", "No");
        }
    } else if (field->id == XYM_UINT8_MD_TRANS_FLAG) {
        if ((value & 0x02) != 0) {
            SNPRINTF(dst, "%s", "Yes");
        } else {
            SNPRINTF(dst, "%s", "No");
        }
    } else if (field->id == XYM_UINT8_MD_RESTRICT_FLAG) {
        if ((value & 0x04) != 0) {
            SNPRINTF(dst, "%s", "Yes");
        } else {
            SNPRINTF(dst, "%s", "No");
        }
    } else {
        SNPRINTF(dst, "%d", value);
    }
}

static void uint8_custom_formatter(const field_t *field, char *dst) {
    uint8_t value = read_uint8(field->data);
    if (value != 0) {
        if (field->id == XYM_UINT8_AA_RESTRICTION) {
            SNPRINTF(dst, "%d %s", value, "address(es)");
        } else if (field->id == XYM_UINT8_AM_RESTRICTION) {
            SNPRINTF(dst, "%d %s", value, "mosaic(s)");
        } else if (field->id == XYM_UINT8_AO_RESTRICTION) {
            SNPRINTF(dst, "%d %s", value, "operation(s)");
        }
    } else {
        SNPRINTF(dst, "%s", "Not change");
    }
}

static void uint16_formatter(const field_t *field, char *dst) {
    uint16_t value = read_uint16(field->data);
    if (field->id == XYM_UINT16_AR_RESTRICT_TYPE) {
        if ((value & 0x0001) != 0) {
            SNPRINTF(dst, "%s", "Address");
        } else if ((value & 0x0002) != 0) {
            SNPRINTF(dst, "%s", "Mosaic");
        } else if ((value & 0x0004) != 0) {
            SNPRINTF(dst, "%s", "Transaction Type");
        }
    } else if (field->id == XYM_UINT16_AR_RESTRICT_DIRECTION) {
        if ((value & 0x4000) != 0) {
            SNPRINTF(dst, "%s", "Outgoing");
        } else {
            SNPRINTF(dst, "%s", "Imcoming");
        }
    } else if (field->id == XYM_UINT16_AR_RESTRICT_OPERATION) {
        if ((value & 0x8000) != 0) {
            SNPRINTF(dst, "%s", "Block");
        } else {
            SNPRINTF(dst, "%s", "Allow");
        }
    } else {
        switch (value) {
            CASE_FIELDVALUE(XYM_TXN_TRANSFER, "Transfer")
            CASE_FIELDVALUE(XYM_TXN_REGISTER_NAMESPACE, "Namespace Registration")
            CASE_FIELDVALUE(XYM_TXN_ACCOUNT_METADATA, "Account Metadata")
            CASE_FIELDVALUE(XYM_TXN_MOSAIC_METADATA, "Mosaic Metadata")
            CASE_FIELDVALUE(XYM_TXN_NAMESPACE_METADATA, "Namespace Metadata")
            CASE_FIELDVALUE(XYM_TXN_ADDRESS_ALIAS, "Address Alias")
            CASE_FIELDVALUE(XYM_TXN_MOSAIC_ALIAS, "Mosaic Alias")
            CASE_FIELDVALUE(XYM_TXN_ACCOUNT_ADDRESS_RESTRICTION, "Account Address Restriction")
            CASE_FIELDVALUE(XYM_TXN_ACCOUNT_MOSAIC_RESTRICTION, "Account Mosaic Restriction")
            CASE_FIELDVALUE(XYM_TXN_ACCOUNT_OPERATION_RESTRICTION, "Account Operation Restriction")
            CASE_FIELDVALUE(XYM_TXN_MOSAIC_ADDRESS_RESTRICTION, "Mosaic Address Restriction")
            CASE_FIELDVALUE(XYM_TXN_MOSAIC_GLOBAL_RESTRICTION, "Mosaic Global Restriction")
            CASE_FIELDVALUE(XYM_TXN_MOSAIC_DEFINITION, "Mosaic definition")
            CASE_FIELDVALUE(XYM_TXN_MOSAIC_SUPPLY_CHANGE, "Mosaic Supply Change")
            CASE_FIELDVALUE(XYM_TXN_MODIFY_MULTISIG_ACCOUNT, "Multisig Account Modification")
            CASE_FIELDVALUE(XYM_TXN_ACCOUNT_KEY_LINK, "Account Key Link")
            CASE_FIELDVALUE(XYM_TXN_NODE_KEY_LINK, "Node Key Link")
            CASE_FIELDVALUE(XYM_TXN_VOTING_KEY_LINK, "Voting Key Link")
            CASE_FIELDVALUE(XYM_TXN_VRF_KEY_LINK, "Vrf Key Link")
            CASE_FIELDVALUE(XYM_TXN_AGGREGATE_COMPLETE, "Aggregate Complete")
            CASE_FIELDVALUE(XYM_TXN_AGGREGATE_BONDED, "Aggregate Bonded")
            CASE_FIELDVALUE(XYM_TXN_FUND_LOCK, "Funds Lock")
            CASE_FIELDVALUE(XYM_TXN_SECRET_LOCK, "Secret Lock")
            CASE_FIELDVALUE(XYM_TXN_SECRET_PROOF, "Secret Proof")
            default:
                SNPRINTF(dst, "%s", "Unknown");
        }
    }
}

static void uint32_formatter(const field_t *field, char *dst) {
    uint32_t value = read_uint32(field->data);
    if ((field->id == XYM_UINT32_VKL_START_POINT) || (field->id == XYM_UINT32_VKL_END_POINT)) {
        SNPRINTF(dst, "%d", value);
    }
}

static void hash_formatter(const field_t *field, char *dst) {
    snprintf_hex(dst, MAX_FIELD_LEN, field->data, field->length, 0);
}

static void uint64_formatter(const field_t *field, char *dst) {
    if (field->id == XYM_UINT64_DURATION) {
        uint64_t duration = read_uint64(field->data);
        if (duration == 0) {
            SNPRINTF(dst, "%s", "Unlimited");
        } else {
            uint16_t day = duration / 2880;
            uint8_t hour = (duration % 2880) / 120;
            uint8_t min = (duration % 120) / 2;
            SNPRINTF(dst, "%d%s%d%s%d%s", day, "d ", hour, "h ", min, "m");
        }
    } else if (field->id == XYM_UINT64_MSC_AMOUNT) {
        xym_print_amount(read_uint64(field->data), 0, "", dst, MAX_FIELD_LEN);
    } else {
        snprintf_hex(dst, MAX_FIELD_LEN, field->data, field->length, 1);
    }
}

static void address_formatter(const field_t *field, char *dst) {
    base32_encode(field->data, XYM_ADDRESS_LENGTH, dst, XYM_PRETTY_ADDRESS_LENGTH);
    dst[39] = '\0';
}

static void mosaic_formatter(const field_t *field, char *dst) {
    if (field->dataType == STI_MOSAIC_CURRENCY) {
        const mosaic_t *value = (const mosaic_t *) field->data;
        bool is_using_mainnet =
            (transactionContext.bip32Path[1] & 0x7FFFFFFF) == 4343 ? true : false;
        if ((value->mosaicId ==
             (is_using_mainnet ? XYM_MAINNET_MOSAIC_ID : XYM_TESTNET_MOSAIC_ID)) ||
            field->id == XYM_MOSAIC_HL_QUANTITY) {
            xym_print_amount(value->amount, 6, "XYM", dst, MAX_FIELD_LEN);
        } else {
            snprintf_mosaic(dst, MAX_FIELD_LEN, value, (char *) "micro");
        }
    }
}

static void xym_formatter(const field_t *field, char *dst) {
    if (field->dataType == STI_XYM) {
        xym_print_amount(read_uint64(field->data), 6, "XYM", dst, MAX_FIELD_LEN);
    }
}

static void hex_msg_formatter(const field_t *field, char *dst) {
    if (field->length >= MAX_FIELD_LEN / 2 - 1) {
        snprintf_hex2ascii(dst, MAX_FIELD_LEN, &field->data[0], MAX_FIELD_LEN / 2 - 1);
    } else {
        snprintf_hex2ascii(dst, MAX_FIELD_LEN, &field->data[0], field->length);
    }
}

static void msg_formatter(const field_t *field, char *dst) {
    if (field->length == 0) {
        SNPRINTF(dst, "%s", "<empty msg>");
    } else if (field->length >= MAX_FIELD_LEN) {
        snprintf_ascii(dst, MAX_FIELD_LEN, &field->data[0], MAX_FIELD_LEN - 1);
    } else {
        snprintf_ascii(dst, MAX_FIELD_LEN, &field->data[0], field->length);
    }
}

static void string_formatter(const field_t *field, char *dst) {
    if (field->id == XYM_UNKNOWN_MOSAIC) {
        SNPRINTF(dst, "%s", "Divisibility and levy cannot be shown");
    } else if (field->id == XYM_STR_RECIPIENT_ADDRESS) {
        SNPRINTF(dst, "%s", "alias to a namespace");
    } else if (field->length > MAX_FIELD_LEN) {
        snprintf_ascii(dst, MAX_FIELD_LEN, field->data, MAX_FIELD_LEN - 1);
    } else {
        snprintf_ascii(dst, MAX_FIELD_LEN, field->data, field->length);
    }
}

static field_formatter_t get_formatter(const field_t *field) {
    switch (field->dataType) {
        case STI_INT8:
            return int8_formatter;
        case STI_UINT8:
            return uint8_formatter;
        case STI_UINT8_ADDITION:
        case STI_UINT8_DELETION:
            return uint8_custom_formatter;
        case STI_INT16:
            return int16_formatter;
        case STI_UINT16:
            return uint16_formatter;
        case STI_UINT32:
            return uint32_formatter;
        case STI_UINT64:
            return uint64_formatter;
        case STI_HASH256:
            return hash_formatter;
        case STI_PUBLIC_KEY:
            return hash_formatter;
        case STI_ADDRESS:
            return address_formatter;
        case STI_MOSAIC_CURRENCY:
            return mosaic_formatter;
        case STI_XYM:
            return xym_formatter;
        case STI_MESSAGE:
            return msg_formatter;
        case STI_HEX_MESSAGE:
            return hex_msg_formatter;
        case STI_STR:
            return string_formatter;
        default:
            return NULL;
    }
}

void format_field(const field_t *field, char *dst) {
    memset(dst, 0, MAX_FIELD_LEN);

    field_formatter_t formatter = get_formatter(field);
    if (formatter != NULL) {
        formatter(field, dst);
    } else {
        SNPRINTF(dst, "%s", "[Not implemented]");
    }

    // Replace a zero-length string with a space because of rendering issues
    if (dst[0] == 0x00) {
        dst[0] = ' ';
    }
}
