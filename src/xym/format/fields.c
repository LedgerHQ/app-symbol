/*******************************************************************************
 *   XYM Wallet
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
#include "fields.h"
#include "common.h"
#include "limitations.h"

void resolve_fieldname(const field_t* field, char* dst) {
    if (field->dataType == STI_INT8) {
        switch (field->id) {
            CASE_FIELDNAME(XYM_INT8_MAM_REMOVAL_DELTA, "Min Removal")
            CASE_FIELDNAME(XYM_INT8_MAM_APPROVAL_DELTA, "Min Approval")
        }
    }

    if (field->dataType == STI_UINT8) {
        switch (field->id) {
            CASE_FIELDNAME(XYM_UINT8_TXN_MESSAGE_TYPE, "Message Type")
            CASE_FIELDNAME(XYM_UINT8_MOSAIC_COUNT, "Mosaics")
            CASE_FIELDNAME(XYM_UINT8_MSC_ACTION, "Change Direction")
            CASE_FIELDNAME(XYM_UINT8_NS_REG_TYPE, "Namespace Type")
            CASE_FIELDNAME(XYM_UINT8_AA_TYPE, "Alias Type")
            CASE_FIELDNAME(XYM_UINT8_MD_DIV, "Divisibility")
            CASE_FIELDNAME(XYM_UINT8_KL_TYPE, "Action")
            CASE_FIELDNAME(XYM_UINT8_MD_TRANS_FLAG, "Transferable")
            CASE_FIELDNAME(XYM_UINT8_MD_SUPPLY_FLAG, "Supply Mutable")
            CASE_FIELDNAME(XYM_UINT8_MD_RESTRICT_FLAG, "Restrictable")
            CASE_FIELDNAME(XYM_UINT8_MAM_ADD_COUNT, "Address Add Num")
            CASE_FIELDNAME(XYM_UINT8_MAM_DEL_COUNT, "Address Del Num")
        }
    }

    if (field->dataType == STI_INT16) {
        switch (field->id) { CASE_FIELDNAME(XYM_INT16_VALUE_DELTA, "Value Size Delta") }
    }

    if (field->dataType == STI_UINT16) {
        switch (field->id) {
            CASE_FIELDNAME(XYM_UINT16_TRANSACTION_TYPE, "Transaction Type")
            CASE_FIELDNAME(XYM_UINT16_INNER_TRANSACTION_TYPE, "Inner TX Type")
            CASE_FIELDNAME(XYM_UINT16_TRANSACTION_DETAIL_TYPE, "Detail TX Type")
            CASE_FIELDNAME(XYM_UINT16_ENTITY_RESTRICT_OPERATION, "Operation Type")
            CASE_FIELDNAME(XYM_UINT16_AR_RESTRICT_TYPE, "Restriction Flag")
            CASE_FIELDNAME(XYM_UINT16_AR_RESTRICT_DIRECTION, "Restriction Flag")
            CASE_FIELDNAME(XYM_UINT16_AR_RESTRICT_OPERATION, "Restriction Flag")
        }
    }

    if (field->dataType == STI_UINT32) {
        switch (field->id) {
            CASE_FIELDNAME(XYM_UINT32_VKL_START_POINT, "Start point")
            CASE_FIELDNAME(XYM_UINT32_VKL_END_POINT, "End point")
        }
    }

    if (field->dataType == STI_UINT64) {
        switch (field->id) {
            CASE_FIELDNAME(XYM_UINT64_DURATION, "Duration")
            CASE_FIELDNAME(XYM_UINT64_PARENTID, "Parent ID")
            CASE_FIELDNAME(XYM_UINT64_MSC_AMOUNT, "Change Amount")
            CASE_FIELDNAME(XYM_UINT64_NS_ID, "Namespace ID")
            CASE_FIELDNAME(XYM_UINT64_MOSAIC_ID, "Mosaic ID")
            CASE_FIELDNAME(XYM_UINT64_METADATA_KEY, "Metadata Key")
        }
    }

    if (field->dataType == STI_HASH256) {
        switch (field->id) {
            CASE_FIELDNAME(XYM_HASH256_AGG_HASH, "Agg. Tx Hash")
            CASE_FIELDNAME(XYM_HASH256_HL_HASH, "Tx Hash")
        }
    }

    if (field->dataType == STI_PUBLIC_KEY) {
        switch (field->id) {
            CASE_FIELDNAME(XYM_PUBLICKEY_ACCOUNT_KEY_LINK, "Linked Acct. PbK")
            CASE_FIELDNAME(XYM_PUBLICKEY_NODE_KEY_LINK, "Linked Node PbK")
            CASE_FIELDNAME(XYM_PUBLICKEY_VOTING_KEY_LINK, "Linked Vot. PbK")
            CASE_FIELDNAME(XYM_PUBLICKEY_VRF_KEY_LINK, "Linked Vrf PbK")
        }
    }

    if (field->dataType == STI_ADDRESS) {
        switch (field->id) {
            CASE_FIELDNAME(XYM_STR_RECIPIENT_ADDRESS, "Recipient")
            CASE_FIELDNAME(XYM_STR_METADATA_ADDRESS, "Target Address")
            CASE_FIELDNAME(XYM_STR_ADDRESS, "Address")
        }
    }

    if (field->dataType == STI_MOSAIC_CURRENCY) {
        switch (field->id) {
            CASE_FIELDNAME(XYM_MOSAIC_AMOUNT, "Amount")
            CASE_FIELDNAME(XYM_MOSAIC_HL_QUANTITY, "Lock Quantity")
        }
    }

    if (field->dataType == STI_XYM) {
        switch (field->id) { CASE_FIELDNAME(XYM_UINT64_TXN_FEE, "Fee") }
    }

    if (field->dataType == STI_MESSAGE) {
        switch (field->id) {
            CASE_FIELDNAME(XYM_STR_TXN_MESSAGE, "Message")
            CASE_FIELDNAME(XYM_STR_METADATA_VALUE, "Value")
        }
    }

    if (field->dataType == STI_HEX_MESSAGE) {
        switch (field->id) {
            CASE_FIELDNAME(XYM_STR_TXN_HARVESTING, "Harvesting Message")
            CASE_FIELDNAME(XYM_STR_TXN_HARVESTING_1, "Harvest. Msg 1")
            CASE_FIELDNAME(XYM_STR_TXN_HARVESTING_2, "Harvest. Msg 2")
            CASE_FIELDNAME(XYM_STR_TXN_HARVESTING_3, "Harvest. Msg 3")
        }
    }

    if (field->dataType == STI_STR) {
        switch (field->id) {
            CASE_FIELDNAME(XYM_UNKNOWN_MOSAIC, "Unknown Mosaic")
            CASE_FIELDNAME(XYM_STR_RECIPIENT_ADDRESS, "Recipient")
            CASE_FIELDNAME(XYM_STR_NAMESPACE, "Name")
        }
    }

    if (field->dataType == STI_UINT8_ADDITION) {
        switch (field->id) {
            CASE_FIELDNAME(XYM_UINT8_AA_RESTRICTION, "Addition Count")
            CASE_FIELDNAME(XYM_UINT8_AM_RESTRICTION, "Addition Count")
            CASE_FIELDNAME(XYM_UINT8_AO_RESTRICTION, "Addition Count")
        }
    }

    if (field->dataType == STI_UINT8_DELETION) {
        switch (field->id) {
            CASE_FIELDNAME(XYM_UINT8_AA_RESTRICTION, "Deletion Count")
            CASE_FIELDNAME(XYM_UINT8_AM_RESTRICTION, "Deletion Count")
            CASE_FIELDNAME(XYM_UINT8_AO_RESTRICTION, "Deletion Count")
        }
    }

    // Default case
    snprintf(dst, MAX_FIELDNAME_LEN, "Unknown Field");
}
