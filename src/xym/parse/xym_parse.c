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

#include "xym_parse.h"
#include "apdu/global.h"
#include "xym/format/printers.h"

#pragma pack(push, 1)

typedef struct {
    uint8_t recipientAddress[XYM_ADDRESS_LENGTH];
    uint16_t messageSize;
    uint8_t mosaicsCount;
    uint32_t reserved1;
    uint8_t reserved2;
} txn_header_t;

typedef struct {
    uint64_t mosaicId;
    uint64_t duration;
    uint32_t nonce;
    uint8_t flags;
    uint8_t divisibility;
} mosaic_definition_data_t;

typedef struct {
    mosaic_t mosaic;
    uint8_t action;
} mosaic_supply_change_data_t;

typedef struct {
    uint32_t size;
    uint32_t reserve1;
    uint8_t signerPublicKey[XYM_PUBLIC_KEY_LENGTH];
    uint32_t reserve2;
    uint8_t version;
    uint8_t network;
    uint16_t innerTxType;
} inner_tx_header_t;

typedef struct {
    uint8_t transactionHash[XYM_TRANSACTION_HASH_LENGTH];
    uint32_t payloadSize;
    uint32_t reserse;
} aggregate_txn_t;

typedef struct {
    uint64_t duration;
    uint64_t namespaceId;
    uint8_t registrationType;
    uint8_t nameSize;
} ns_header_t;

typedef struct {
    uint8_t address[XYM_ADDRESS_LENGTH];
    uint64_t metadataKey;
} metadata_header_1_t;

typedef struct {
    int16_t valueSizeDelta;
    uint16_t valueSize;
} metadata_header_2_t; 

typedef struct {
    metadata_header_1_t address_data;
    metadata_header_2_t value_data;
} am_header_t;

typedef struct {
    metadata_header_1_t address_data;
    uint64_t mosaicNamespaceId;
    metadata_header_2_t value_data;
} mnm_header_t;

typedef struct {
    uint64_t namespaceId;
    uint8_t address[XYM_ADDRESS_LENGTH];
    uint8_t aliasAction;
} aa_header_t;

typedef struct {
    uint64_t namespaceId;
    uint64_t mosaicId;
    uint8_t aliasAction;
} ma_header_t;

typedef struct {
    uint16_t restrictionFlags;
    uint8_t restrictionAdditionsCount;
    uint8_t restrictionDeletionsCount;
    uint32_t reserve;
} ar_header_t;

typedef struct {
    uint8_t linkedPublicKey[XYM_PUBLIC_KEY_LENGTH];
    uint8_t linkAction;
} key_link_header_t;

typedef struct {
    uint8_t linkedPublicKey[XYM_PUBLIC_KEY_LENGTH];
    uint32_t startPoint;
    uint32_t endPoint;
    uint8_t linkAction;
} voting_key_link_header_t;

typedef struct {
    int8_t minRemovalDelta;
    int8_t minApprovalDelta;
    uint8_t addressAdditionsCount;
    uint8_t addressDeletionsCount;
    uint32_t reserve;
} multisig_account_t;

typedef struct {
    mosaic_t mosaic;
    uint64_t blockDuration;
    uint8_t aggregateBondedHash[XYM_TRANSACTION_HASH_LENGTH];
} fl_header_t;

typedef struct {
    uint8_t transactionHash[XYM_TRANSACTION_HASH_LENGTH];
    uint8_t version;
    uint8_t networkType;
    uint16_t transactionType;
} common_header_t;

#pragma pack(pop)

#define BAIL_IF(x) {int err = x; if (err) return err;}


static int add_new_field( fields_array_t* fields, uint8_t id, uint8_t data_type, uint32_t length, const uint8_t* data )
{
    uint8_t idx = fields->numFields;

    if( idx >= MAX_FIELD_COUNT ) { return E_TOO_MANY_FIELDS; }
    if( data == NULL           ) { return E_NOT_ENOUGH_DATA; }

    field_t* field = &fields->arr[idx];
    field->id       = id;
    field->dataType = data_type;
    field->length   = length;
    field->data     = data;

    fields->numFields++;

    return E_SUCCESS;
}




/**
 * TransferTransaction:
 * https://docs.symbolplatform.com/serialization/transfer.html#transfertransaction
 * 
 * 
 * Input data (rawTxData)
 * ----------------------------------
 *      uint64_t maxFee;
 *      uint64_t deadline;
 * 
 *      uint8_t  recipientAddress[ XYM_ADDRESS_LENGTH ];
 *      uint16_t messageSize;
 *      uint8_t  mosaicsCount;
 *      uint32_t reserved1;
 *      uint8_t  reserved2;
 * 
 *      mosaic_t mosaics[ mosaicsCount ];
 *      uint8_t  message[ messageSize  ];
 * ----------------------------------
 *
 * 
 * Output (fields)
 * ----------------------------------
 *      recipientAddress
 *      mosaicsCount
 *      mosaic              ///< 'mosaicsCount' times
 *      message
 * 
 *      maxFee (only if not multisig)
 */
static int parse_transfer_txn_content( buffer_t* rawTxData, fields_array_t* fields )
{
    // get header
    const txn_header_t *txn = (const txn_header_t*) buffer_offset_ptr_and_seek( rawTxData, sizeof(txn_header_t)); // Read data and security check
    if( !txn ) { return E_NOT_ENOUGH_DATA; }

    uint32_t length = txn->mosaicsCount * sizeof(mosaic_t) + txn->messageSize;
    if( !buffer_can_read(rawTxData, length) ) { return E_INVALID_DATA; } 

    if( txn->recipientAddress[0] == MAINNET_NETWORK_TYPE || txn->recipientAddress[0] == TESTNET_NETWORK_TYPE ) 
    {
        BAIL_IF( add_new_field(fields, XYM_STR_RECIPIENT_ADDRESS, STI_ADDRESS, XYM_ADDRESS_LENGTH, (const uint8_t*) txn->recipientAddress) ); // add recipient address
    } 
    else 
    {        
        BAIL_IF( add_new_field(fields, XYM_STR_RECIPIENT_ADDRESS, STI_STR,    0,                (const uint8_t*) &txn->recipientAddress[0]) ); // add recipient alias to namespace notification
        BAIL_IF( add_new_field(fields, XYM_UINT64_NS_ID,          STI_UINT64, sizeof(uint64_t), (const uint8_t*) &txn->recipientAddress[1]) ); // add alias namespace ID
    }
    
    if( txn->mosaicsCount > 1 )
    {
        BAIL_IF( add_new_field(fields, XYM_UINT8_MOSAIC_COUNT, STI_UINT8, sizeof(uint8_t), (const uint8_t*) &txn->mosaicsCount) ); // add sent mosaic count field
    }

    const bool     is_using_mainnet = (transactionContext.bip32Path[1] & 0x7FFFFFFF) == 4343; // checks if the coin_type field of bip32 path is 'symbol'
    const uint64_t mosaic_net_id    = (is_using_mainnet ? XYM_MAINNET_MOSAIC_ID : XYM_TESTNET_MOSAIC_ID);

    // Show mosaics amounts
    for( uint8_t i = 0; i < txn->mosaicsCount; i++ ) 
    {
        const mosaic_t* mosaic = (const mosaic_t*) buffer_offset_ptr_and_seek( rawTxData, sizeof(mosaic_t));
        if( !mosaic ){ return E_NOT_ENOUGH_DATA; }


        if( txn->mosaicsCount == 1 && mosaic->mosaicId != mosaic_net_id ) 
        {
            // Show sent mosaic count field (only 1 unknown mosaic)
            BAIL_IF( add_new_field(fields, XYM_UINT8_MOSAIC_COUNT, STI_UINT8, sizeof(uint8_t), (const uint8_t*) &txn->mosaicsCount) ); 
        }

        if( mosaic->mosaicId != mosaic_net_id )
        {            
            BAIL_IF( add_new_field(fields, XYM_UNKNOWN_MOSAIC, STI_STR, 0, (const uint8_t*) mosaic) ); // Unknow mosaic notification
        }

        BAIL_IF( add_new_field(fields, XYM_MOSAIC_AMOUNT, STI_MOSAIC_CURRENCY, sizeof(mosaic_t), (const uint8_t*) mosaic) );
    }

    if( txn->messageSize == 0 ) 
    {
        // Show Empty Message
        BAIL_IF(add_new_field(fields, XYM_STR_TXN_MESSAGE, STI_MESSAGE, txn->messageSize, (const uint8_t*) &txn->messageSize));
    }
    else 
    {
        // first byte of message is the message type
        if( !buffer_can_read(rawTxData, sizeof(uint8_t)) ) { return E_INVALID_DATA; }
        const uint8_t* msgType = buffer_offset_ptr( rawTxData );
        BAIL_IF(add_new_field(fields, XYM_UINT8_TXN_MESSAGE_TYPE, STI_UINT8, sizeof(uint8_t), msgType)); // Show Message Type

        if (*msgType == XYM_PERSISTENT_DELEGATED_HARVESTING) // TODO: just do one read in a new buffer_t above, and use that below, instead of doing multiple seeks
        {
            // Show persistent harvesting delegation message
        #if defined(TARGET_NANOX) || defined(TARGET_NANOS2)
            BAIL_IF( add_new_field(fields, XYM_STR_TXN_HARVESTING, STI_HEX_MESSAGE, txn->messageSize,      buffer_offset_ptr_and_seek( rawTxData, txn->messageSize)) );
        #elif defined(TARGET_NANOS)
            BAIL_IF( add_new_field(fields, XYM_STR_TXN_HARVESTING_1, STI_HEX_MESSAGE, MAX_FIELD_LEN/2 - 1,                  buffer_offset_ptr_and_seek( rawTxData, MAX_FIELD_LEN/2 - 1)) ); 
            BAIL_IF( add_new_field(fields, XYM_STR_TXN_HARVESTING_2, STI_HEX_MESSAGE, MAX_FIELD_LEN/2 - 1,                  buffer_offset_ptr_and_seek( rawTxData, MAX_FIELD_LEN/2 - 1)) ); 
            BAIL_IF( add_new_field(fields, XYM_STR_TXN_HARVESTING_3, STI_HEX_MESSAGE, txn->messageSize - MAX_FIELD_LEN + 2, buffer_offset_ptr_and_seek( rawTxData, txn->messageSize - MAX_FIELD_LEN + 2)) ); 
        #endif
        }
        else 
        {
            if( !buffer_seek(rawTxData, 1) ){ return E_NOT_ENOUGH_DATA; } // Message type            
            BAIL_IF( add_new_field(fields, XYM_STR_TXN_MESSAGE, STI_MESSAGE, txn->messageSize - 1, buffer_offset_ptr_and_seek( rawTxData, txn->messageSize - 1)) ); // Show Message in plain text
        }
    }

    return E_SUCCESS;
}

/**
 * MosaicDefinitionTransaction:
 * https://docs.symbolplatform.com/serialization/mosaic.html#mosaicdefinitiontransaction
 * 
 * 
 * Input data (rawTxData)
 * ----------------------------------
 * fee (only if multisig)
 * {
 *     uint64_t maxFee;
 *     uint64_t deadline;
 * }
 * 
 * transaction data 
 * {
 *      uint64_t mosaicId;
 *      uint64_t duration;
 *      uint32_t nonce;
 *      uint8_t flags;
 *      uint8_t divisibility;
 * }
 * ----------------------------------
 *
 * 
 * Output fields
 * -----------------------------------
 * fields
 * {
 *      mosaicId
 *      divisibility
 *      duration
 *      flag transferable
 *      flag supply mutable
 *      flag restrictable
 * 
 *      maxFee (only if multisig)
 * }
 */
static int parse_mosaic_definition_txn_content( buffer_t* rawTxData, fields_array_t* fields ) 
{
    const mosaic_definition_data_t *txn = (const mosaic_definition_data_t*) buffer_offset_ptr_and_seek( rawTxData, sizeof(mosaic_definition_data_t) ); // Read data and security check
    if( !txn ) { return E_NOT_ENOUGH_DATA; }

    BAIL_IF( add_new_field(fields, XYM_UINT64_MOSAIC_ID,       STI_UINT64, sizeof(uint64_t), (const uint8_t*) &txn->mosaicId)     ); // Show mosaic id
    BAIL_IF( add_new_field(fields, XYM_UINT8_MD_DIV,           STI_UINT8,  sizeof(uint8_t),  (const uint8_t*) &txn->divisibility) ); // Show mosaic divisibility
    BAIL_IF( add_new_field(fields, XYM_UINT64_DURATION,        STI_UINT64, sizeof(uint64_t), (const uint8_t*) &txn->duration)     ); // Show duration
    BAIL_IF( add_new_field(fields, XYM_UINT8_MD_TRANS_FLAG,    STI_UINT8,  sizeof(uint8_t),  (const uint8_t*) &txn->flags)        ); // Show mosaic flag (Transferable)
    BAIL_IF( add_new_field(fields, XYM_UINT8_MD_SUPPLY_FLAG,   STI_UINT8,  sizeof(uint8_t),  (const uint8_t*) &txn->flags)        ); // Show mosaic flag (Supply mutable)
    BAIL_IF( add_new_field(fields, XYM_UINT8_MD_RESTRICT_FLAG, STI_UINT8,  sizeof(uint8_t),  (const uint8_t*) &txn->flags)        ); // Show mosaic flag (Restrictable)

    return E_SUCCESS;
}



/**
 * MosaicSupplyChangeTransaction
 * https://docs.symbolplatform.com/serialization/mosaic.html#mosaic-supply-change
 * 
 * transaction data (rawTxData)
 * ----------------------------------
 * {
 *      uint64_t maxFee;    ///< (only if multisig)
 *      uint64_t deadline;  ///< (only if multisig)
 * 
 *      uint64_t mosaicId;
 *      uint64_t amount;
 *      uint8_t  action;
 * }
 * 
 * Output (fields)
 * -----------------------------------
 * {
 *      mosaicId,
 *      action,
 *      amount,
 * 
 *      maxFee (only if multisig)
 * }
 */
static int parse_mosaic_supply_change_txn_content( buffer_t* rawTxData, fields_array_t* fields )
{
    const mosaic_supply_change_data_t *txn = (const mosaic_supply_change_data_t*) buffer_offset_ptr_and_seek( rawTxData, sizeof(mosaic_supply_change_data_t) );
    if( !txn ) { return E_NOT_ENOUGH_DATA; }
   
    BAIL_IF( add_new_field(fields, XYM_UINT64_MOSAIC_ID,  STI_UINT64, sizeof(uint64_t), (const uint8_t*) &txn->mosaic.mosaicId) ); // Show mosaic id
    BAIL_IF( add_new_field(fields, XYM_UINT8_MSC_ACTION,  STI_UINT8,  sizeof(uint8_t),  (const uint8_t*) &txn->action)          ); // Show supply change action
    BAIL_IF( add_new_field(fields, XYM_UINT64_MSC_AMOUNT, STI_UINT64, sizeof(mosaic_t), (const uint8_t*) &txn->mosaic.amount)   ); // Show amount
    
    return E_SUCCESS;
}



/**
 * MultisigAccountModificationTransaction
 * https://docs.symbolplatform.com/serialization/multisig.html#multisig-account-modification
 * 
 * 
 * transaction data (rawTxData)
 * ----------------------------------
 * {
 *      uint64_t maxFee;    ///< (only if multisig)
 *      uint64_t deadline;  ///< (only if multisig)
 * 
 *      int8_t minRemovalDelta;
 *      int8_t minApprovalDelta;
 *      uint8_t addressAdditionsCount;
 *      uint8_t addressDeletionsCount;
 *      uint32_t reserve;
 *      addressAdditions[addressAdditionsCount];
 *      addressDeletions[addressDeletionsCount];
 * }
 * 
 * Output (fields)
 * -----------------------------------
 * {
 *      addressAdditionsCount,
 *      addressAdditions,  // shown 'addressAdditionsCount' times
 * 
 *      addressDeletionsCount,
 *      addressDeletions,  // shown 'addressDeletionsCount' times
 * 
 *      minApprovalDelta,
 *      minRemovalDelta,
 * 
 *      maxFee //(only if multisig)
 * }
 */
static int parse_multisig_account_modification_txn_content( buffer_t* rawTxData, fields_array_t* fields )
{
    // get header
    const multisig_account_t *txn = (const multisig_account_t*) buffer_offset_ptr_and_seek( rawTxData, sizeof(multisig_account_t)); // Read data and security check
    if( !txn ) { return E_NOT_ENOUGH_DATA; }

    // Show address additions count
    BAIL_IF(add_new_field(fields, XYM_UINT8_MAM_ADD_COUNT, STI_UINT8, sizeof(uint8_t), (const uint8_t*) &txn->addressAdditionsCount)); 

    // Show list of addition address
    for( uint8_t i = 0; i < txn->addressAdditionsCount; i++ )
    {
        BAIL_IF( add_new_field(fields, XYM_STR_ADDRESS, STI_ADDRESS, XYM_ADDRESS_LENGTH, buffer_offset_ptr_and_seek(rawTxData, XYM_ADDRESS_LENGTH)) );
    }
    
    // Show address deletions count
    BAIL_IF( add_new_field(fields, XYM_UINT8_MAM_DEL_COUNT, STI_UINT8, sizeof(uint8_t), (const uint8_t*) &txn->addressDeletionsCount) );

    // Show list of addition address
    for (uint8_t i = 0; i < txn->addressDeletionsCount; i++) 
    {
        BAIL_IF(add_new_field(fields, XYM_STR_ADDRESS, STI_ADDRESS, XYM_ADDRESS_LENGTH, buffer_offset_ptr_and_seek(rawTxData, XYM_ADDRESS_LENGTH))); // Read data and security check
    }
    
    // Show min approval delta
    BAIL_IF(add_new_field(fields, XYM_INT8_MAM_APPROVAL_DELTA, STI_INT8, sizeof(int8_t), (const uint8_t*) &txn->minApprovalDelta));

    // Show min removal delta
    BAIL_IF(add_new_field(fields, XYM_INT8_MAM_REMOVAL_DELTA, STI_INT8, sizeof(int8_t), (const uint8_t*) &txn->minRemovalDelta));
    
    return E_SUCCESS;
}


/**
 * NamespaceRegistrationTransaction
 * https://docs.symbolplatform.com/serialization/namespace.html#namespace-registration
 * 
 * 
 * transaction data (rawTxData)
 * ----------------------------------
 * {
 *      uint64_t maxFee;    ///< (only if multisig)
 *      uint64_t deadline;  ///< (only if multisig)
 * 
 *      uint64_t duration;
 *      uint64_t namespaceId;
 *      uint8_t  registrationType;
 *      uint8_t  nameSize;
 *      uint8_t* name[nameSize];
 * }
 * 
 * Output (fields)
 * -----------------------------------
 * {
 *      registrationType,
 *      name,
 *      duration,
 * 
 *      maxFee //(only if multisig)
 * }
 */
static int parse_namespace_registration_txn_content( buffer_t* rawTxData, fields_array_t* fields )
{
    // get header
    const ns_header_t *txn = (const ns_header_t*) buffer_offset_ptr_and_seek( rawTxData, sizeof(ns_header_t)); // Read data and security check
    if( !txn ) { return E_NOT_ENOUGH_DATA; }
    
    // extract namespace name
    const uint8_t* namespaceName = buffer_offset_ptr_and_seek( rawTxData, txn->nameSize );
    if( !namespaceName ) { return E_NOT_ENOUGH_DATA; }

    // create fields from extracted data
    const uint8_t fieldId = ( (txn->registrationType==0) ? XYM_UINT64_DURATION : XYM_UINT64_PARENTID );

    BAIL_IF( add_new_field(fields, XYM_UINT8_NS_REG_TYPE, STI_UINT8,  sizeof(uint8_t),  (const uint8_t*) &txn->registrationType) ); // namespace reg type
    BAIL_IF( add_new_field(fields, XYM_STR_NAMESPACE,     STI_STR,    txn->nameSize,                namespaceName)               ); // namespace/sub-namespace name
    BAIL_IF( add_new_field(fields, fieldId,               STI_UINT64, sizeof(uint64_t), (const uint8_t*) &txn->duration)         ); // duration/parentID
    
    return E_SUCCESS;
}


/**
 * AccountMetadataTransaction
 * https://docs.symbolplatform.com/serialization/metadata.html#accountmetadatatransaction
 * 
 * 
 * transaction data (rawTxData)
 * ----------------------------------
 * {
 *      uint64_t maxFee;    ///< (only if not multisig)
 *      uint64_t deadline;  ///< (only if not multisig)
 * 
 *      uint8_t  address[ XYM_ADDRESS_LENGTH ];
 *      uint64_t metadataKey;
 *      int16_t  valueSizeDelta;
 *      uint16_t valueSize;
 *      uint8_t  value[ valueSize ];
 * }
 * 
 * Output (fields)
 * -----------------------------------
 * {
 *      address,
 *      metadataKey,
 *      value,
 *      valueSizeDelta,
 * 
 *      maxFee //(only if not multisig)
 * }
 */
static int parse_account_metadata_txn_content( buffer_t* rawTxData, fields_array_t* fields )
{
    // get header
    const am_header_t *txn = (const am_header_t*) buffer_offset_ptr_and_seek(rawTxData, sizeof(am_header_t));  // get fee
    if( !txn ) { return E_NOT_ENOUGH_DATA; }

    // create fields from extracted data
    BAIL_IF( add_new_field(fields, XYM_STR_METADATA_ADDRESS, STI_ADDRESS, XYM_ADDRESS_LENGTH,        (const uint8_t*) &txn->address_data.address)                      ); // Show metadata target address
    BAIL_IF( add_new_field(fields, XYM_UINT64_METADATA_KEY,  STI_UINT64,  sizeof(uint64_t),          (const uint8_t*) &txn->address_data.metadataKey)                  ); // Show scope metadata key
    BAIL_IF( add_new_field(fields, XYM_STR_METADATA_VALUE,   STI_MESSAGE, txn->value_data.valueSize, buffer_offset_ptr_and_seek(rawTxData, txn->value_data.valueSize)) ); // Show different value
    BAIL_IF( add_new_field(fields, XYM_INT16_VALUE_DELTA,    STI_INT16,   sizeof(uint16_t),          (const uint8_t*) &txn->value_data.valueSizeDelta)                 ); // Show value size delta
    
    return E_SUCCESS;
}


/**
 * MosaicMetadataTransaction and NamespaceMetadataTransaction
 * https://docs.symbolplatform.com/serialization/metadata.html#mosaicmetadatatransaction
 * https://docs.symbolplatform.com/serialization/metadata.html#namespacemetadatatransaction
 * 
 * 
 * transaction data (rawTxData)
 * ----------------------------------
 * {
 *      uint64_t maxFee;    ///< (only if not multisig)
 *      uint64_t deadline;  ///< (only if not multisig)
 * 
 *      uint8_t  address[ XYM_ADDRESS_LENGTH ];
 *      uint64_t metadataKey;
 *      uint64_t mosaicNamespaceId;
 *      int16_t  valueSizeDelta;
 *      uint16_t valueSize;
 *      uint8_t  value[ valueSize ];
 * }
 * 
 * Output (fields)
 * -----------------------------------
 * {
 *      address,
 *      mosaicNamespaceId,
 *      metadataKey,
 *      value,
 *      valueSizeDelta,
 * 
 *      maxFee //(only if not multisig)
 * }
 */
static int parse_metadata_txn_content( buffer_t* rawTxData, uint8_t id, fields_array_t* fields )
{
    // get header    
    const mnm_header_t* txn = (const mnm_header_t*) buffer_offset_ptr_and_seek( rawTxData, sizeof(mnm_header_t));
    if( !txn ) { return E_NOT_ENOUGH_DATA; }

    // get value
    const uint8_t* value = buffer_offset_ptr_and_seek(rawTxData, txn->value_data.valueSize);
    if( !value ) { return E_NOT_ENOUGH_DATA; }
    
    // create fields from extracted data
    BAIL_IF( add_new_field(fields, XYM_STR_METADATA_ADDRESS, STI_ADDRESS, XYM_ADDRESS_LENGTH,        (const uint8_t*) &txn->address_data.address)      ); // add metadata target address
    BAIL_IF( add_new_field(fields, id,                       STI_UINT64,  sizeof(uint64_t),          (const uint8_t*) &txn->mosaicNamespaceId)         ); // add target mosaic/namespace id
    BAIL_IF( add_new_field(fields, XYM_UINT64_METADATA_KEY,  STI_UINT64,  sizeof(uint64_t),          (const uint8_t*) &txn->address_data.metadataKey)  ); // add scope metadata key
    BAIL_IF( add_new_field(fields, XYM_STR_METADATA_VALUE,   STI_MESSAGE, txn->value_data.valueSize, value)                                            ); // add different value
    BAIL_IF( add_new_field(fields, XYM_INT16_VALUE_DELTA,    STI_INT16,   sizeof(uint16_t),          (const uint8_t*) &txn->value_data.valueSizeDelta) ); // add value size delta
    
    return E_SUCCESS;
}
 
static int parse_mosaic_metadata_txn_content( buffer_t* rawTxData, fields_array_t* fields )
{
    return parse_metadata_txn_content( rawTxData, XYM_UINT64_MOSAIC_ID, fields );
}

static int parse_namespace_metadata_txn_content( buffer_t* rawTxData, fields_array_t* fields )
{
    return parse_metadata_txn_content( rawTxData, XYM_UINT64_NS_ID, fields );
}


/**
 * AddressAliasTransaction
 * https://docs.symbolplatform.com/serialization/namespace.html#address-alias-transaction
 * 
 * 
 * transaction data (rawTxData)
 * ----------------------------------
 * {
 *      uint64_t maxFee;    ///< (only if not multisig)
 *      uint64_t deadline;  ///< (only if not multisig)
 * 
 *      uint64_t namespaceId;
 *      uint8_t  address[XYM_ADDRESS_LENGTH];
 *      uint8_t  aliasAction;
 * }
 * 
 * Output (fields)
 * -----------------------------------
 * {
 *      aliasAction,
 *      namespaceId,
 *      address,
 * 
 *      maxFee //(only if not multisig)
 * }
 */
static int parse_address_alias_txn_content( buffer_t* rawTxData, fields_array_t* fields )
{   
    // get header
    const aa_header_t *txn = (const aa_header_t*) buffer_offset_ptr_and_seek( rawTxData, sizeof(aa_header_t)); // Read data and security check
    if( !txn ) { return E_NOT_ENOUGH_DATA; }
    
    // create fields from extracted data
    BAIL_IF( add_new_field(fields, XYM_UINT8_AA_TYPE, STI_UINT8,   sizeof(uint8_t),    (const uint8_t*) &txn->aliasAction) ); // add alias type
    BAIL_IF( add_new_field(fields, XYM_UINT64_NS_ID,  STI_UINT64,  sizeof(uint64_t),   (const uint8_t*) &txn->namespaceId) ); // add namespace id
    BAIL_IF( add_new_field(fields, XYM_STR_ADDRESS,   STI_ADDRESS, XYM_ADDRESS_LENGTH, (const uint8_t*) txn->address)      ); // add Recipient address
    
    return E_SUCCESS;
}




/**
 * MosaicAliasTransaction
 * https://docs.symbolplatform.com/serialization/namespace.html#mosaicaliastransaction
 * 
 * 
 * transaction data (rawTxData)
 * ----------------------------------
 * {
 *      uint64_t maxFee;    ///< (only if not multisig)
 *      uint64_t deadline;  ///< (only if not multisig)
 * 
 *      uint64_t namespaceId;
 *      uint64_t mosaicId;
 *      uint8_t  aliasAction;
 * }
 * 
 * Output (fields)
 * -----------------------------------
 * {
 *      aliasAction,
 *      namespaceId,
 *      mosaicId,
 *      
 * 
 *      maxFee //(only if not multisig)
 * }
 */
static int parse_mosaic_alias_txn_content( buffer_t* rawTxData, fields_array_t* fields )
{
    // get header
    const ma_header_t *txn = (const ma_header_t*) buffer_offset_ptr_and_seek( rawTxData, sizeof(ma_header_t));
    if( !txn ) { return E_NOT_ENOUGH_DATA; }
    
    // create fields from extracted data
    BAIL_IF( add_new_field(fields, XYM_UINT8_AA_TYPE,    STI_UINT8,  sizeof(uint8_t),  (const uint8_t*) &txn->aliasAction) ); // add alisa type
    BAIL_IF( add_new_field(fields, XYM_UINT64_NS_ID,     STI_UINT64, sizeof(uint64_t), (const uint8_t*) &txn->namespaceId) ); // add namespace id
    BAIL_IF( add_new_field(fields, XYM_UINT64_MOSAIC_ID, STI_UINT64, sizeof(uint64_t), (const uint8_t*) &txn->mosaicId)    ); // add mosaic id
    
    return E_SUCCESS;
}




/**
 * AccountAddressRestrictionTransaction, AccountMosaicRestrictionTransaction and AccountOperationRestrictionTransaction
 * --------------------------------------------------------------------------------------------------------------------
 * https://docs.symbolplatform.com/serialization/restriction_account.html#accountaddressrestrictiontransaction
 * https://docs.symbolplatform.com/serialization/restriction_account.html#accountmosaicrestrictiontransaction
 * https://docs.symbolplatform.com/serialization/restriction_account.html#accountoperationrestrictiontransaction
 *
 *
 * transaction data (rawTxData)
 * ----------------------------------
 * {
 *      uint64_t maxFee;    ///< (only if not multisig)
 *      uint64_t deadline;  ///< (only if not multisig)
 * 
 *      uint16_t restrictionFlags;
 *      uint8_t  restrictionAdditionsCount;
 *      uint8_t  restrictionDeletionsCount;
 *      uint32_t reserve;
 * 
 *      uint8_t* restrictionAdditions[ restrictionAdditionsCount ]
 *      uint8_t* restrictionDeletions[ restrictionDeletionsCount ]
 * }
 * 
 * Output (fields)
 * -----------------------------------
 * {
 *      restrictionAdditionsCount,
 *      restrictionAdditions[],
 * 
 *      restrictionDeletionsCount,
 *      restrictionDeletions[],
 *      
 *      restrictionFlags,
 * 
 * 
 *      maxFee //(only if not multisig)
 * }
 */
static int parse_account_restriction_txn_content( buffer_t* rawTxData, uint8_t restrictionType, fields_array_t* fields )
{
    // get header
    const ar_header_t *txn = (const ar_header_t*) buffer_offset_ptr_and_seek( rawTxData, sizeof(ar_header_t)); // Read data and security check
    if( !txn ) { return E_NOT_ENOUGH_DATA; }
    
    // Show address/mosaicId additions count
    BAIL_IF(add_new_field(fields, restrictionType, STI_UINT8_ADDITION, sizeof(uint8_t), (const uint8_t*) &txn->restrictionAdditionsCount));
    
    // Show list of addition address/mosaicId
    for( uint8_t i = 0; i < txn->restrictionAdditionsCount; i++ )
    {
        switch( restrictionType )
        {
            case XYM_UINT8_AA_RESTRICTION:
                BAIL_IF(add_new_field(fields, XYM_STR_ADDRESS,                      STI_ADDRESS, XYM_ADDRESS_LENGTH, buffer_offset_ptr_and_seek(rawTxData, XYM_ADDRESS_LENGTH)) ); 
                break;
            case XYM_UINT8_AM_RESTRICTION:
                BAIL_IF(add_new_field(fields, XYM_UINT64_MOSAIC_ID,                 STI_UINT64, sizeof(uint64_t),    buffer_offset_ptr_and_seek(rawTxData, sizeof(uint64_t)))   ); 
                break;
            case XYM_UINT8_AO_RESTRICTION:
                BAIL_IF(add_new_field(fields, XYM_UINT16_ENTITY_RESTRICT_OPERATION, STI_UINT16, sizeof(uint16_t),    buffer_offset_ptr_and_seek(rawTxData, sizeof(uint16_t)))   ); 
                break;
            default:
                return E_INVALID_DATA;
        }
    }
    
    // Show address/mosaicId deletions count
    BAIL_IF(add_new_field(fields, restrictionType, STI_UINT8_DELETION, sizeof(uint8_t), (const uint8_t*) &txn->restrictionDeletionsCount));
    
    // Show list of addition address
    for( uint8_t i = 0; i < txn->restrictionDeletionsCount; i++ )
    {
        switch (restrictionType) {
            case XYM_UINT8_AA_RESTRICTION:
                BAIL_IF(add_new_field(fields, XYM_STR_ADDRESS,                      STI_ADDRESS, XYM_ADDRESS_LENGTH, buffer_offset_ptr_and_seek(rawTxData, XYM_ADDRESS_LENGTH)) );
                break;
            case XYM_UINT8_AM_RESTRICTION:
                BAIL_IF(add_new_field(fields, XYM_UINT64_MOSAIC_ID,                 STI_UINT64,  sizeof(uint64_t),   buffer_offset_ptr_and_seek(rawTxData, sizeof(uint64_t)))   ); 
                break;
            case XYM_UINT8_AO_RESTRICTION:
                BAIL_IF(add_new_field(fields, XYM_UINT16_ENTITY_RESTRICT_OPERATION, STI_UINT16,  sizeof(uint16_t),   buffer_offset_ptr_and_seek(rawTxData, sizeof(uint16_t)))   ); 
                break;
            default:
                return E_INVALID_DATA;
        }
    }
    
    // Show restriction operation
    BAIL_IF(add_new_field(fields, XYM_UINT16_AR_RESTRICT_OPERATION, STI_UINT16, sizeof(int16_t), (const uint8_t*) &txn->restrictionFlags));
    
    if(restrictionType != XYM_UINT8_AM_RESTRICTION)
    {
        // Show restriction direction
        BAIL_IF(add_new_field(fields, XYM_UINT16_AR_RESTRICT_DIRECTION, STI_UINT16, sizeof(int16_t), (const uint8_t*) &txn->restrictionFlags));
    }
    
    // Show restriction type
    BAIL_IF(add_new_field(fields, XYM_UINT16_AR_RESTRICT_TYPE, STI_UINT16, sizeof(int16_t), (const uint8_t*) &txn->restrictionFlags));
    
    return E_SUCCESS;
}


static int parse_account_address_restriction_txn_content( buffer_t* rawTxData, fields_array_t* fields )
{
    return parse_account_restriction_txn_content(rawTxData, XYM_UINT8_AA_RESTRICTION, fields);
}

static int parse_account_mosaic_restriction_txn_content( buffer_t* rawTxData, fields_array_t* fields )
{
    return parse_account_restriction_txn_content(rawTxData, XYM_UINT8_AM_RESTRICTION, fields);
}

static int parse_account_operation_restriction_txn_content( buffer_t* rawTxData, fields_array_t* fields )
{
    return parse_account_restriction_txn_content(rawTxData, XYM_UINT8_AO_RESTRICTION, fields);
}


/**
 * AccountKeyLinkTransaction, NodeKeyLinkTransaction and VrfKeyLinkTransaction
 * https://docs.symbolplatform.com/serialization/account_link.html#accountkeylinktransaction
 * https://docs.symbolplatform.com/serialization/account_link.html#nodekeylinktransaction
 * https://docs.symbolplatform.com/serialization/coresystem.html#vrf-key-link-transaction
 *
 * transaction data (rawTxData)
 * ----------------------------------
 * {
 *      uint64_t maxFee;    ///< (only if not multisig)
 *      uint64_t deadline;  ///< (only if not multisig)
 * 
 *      uint8_t linkedPublicKey[ XYM_PUBLIC_KEY_LENGTH ];
 *      uint8_t linkAction;
 * }
 * 
 * Output (fields)
 * -----------------------------------
 * {
 *      linkAction,
 *      linkedPublicKey,
 * 
 *      maxFee //(only if not multisig)
 * }
 */
static int parse_key_link_txn_content( buffer_t* rawTxData, uint8_t txType, fields_array_t* fields )
{
    // get header
    const key_link_header_t *txn = (const key_link_header_t*) buffer_offset_ptr_and_seek( rawTxData, sizeof(key_link_header_t)); // Read data and security check
    if( !txn ) { return E_NOT_ENOUGH_DATA; }
    
    // create fields from extracted data
    BAIL_IF( add_new_field(fields, XYM_UINT8_KL_TYPE, STI_UINT8,      sizeof(uint8_t),       (const uint8_t*) &txn->linkAction)      ); // Show link action type
    BAIL_IF( add_new_field(fields, txType,            STI_PUBLIC_KEY, XYM_PUBLIC_KEY_LENGTH, (const uint8_t*) &txn->linkedPublicKey) ); // Show linked public key
    
    return E_SUCCESS;
}

static int parse_account_key_link_txn_content( buffer_t* rawTxData, fields_array_t* fields )
{
    return parse_key_link_txn_content(rawTxData, XYM_PUBLICKEY_ACCOUNT_KEY_LINK, fields);
}

static int parse_node_key_link_txn_content( buffer_t* rawTxData, fields_array_t* fields )
{
    return parse_key_link_txn_content(rawTxData, XYM_PUBLICKEY_NODE_KEY_LINK, fields);
}

static int parse_vrf_key_link_txn_content( buffer_t* rawTxData, fields_array_t* fields )
{
    return parse_key_link_txn_content(rawTxData, XYM_PUBLICKEY_VRF_KEY_LINK, fields);
}




/**
 * VotingKeyLinkTransaction
 * https://docs.symbolplatform.com/serialization/coresystem.html#votingkeylinktransaction
 * transaction data (rawTxData)
 * ----------------------------------
 * {
 *      uint64_t maxFee;    ///< (only if not multisig)
 *      uint64_t deadline;  ///< (only if not multisig)
 * 
 *      uint8_t  linkedPublicKey[XYM_PUBLIC_KEY_LENGTH];
 *      uint32_t startPoint;
 *      uint32_t endPoint;
 *      uint8_t  linkAction;
 * }
 * 
 * Output (fields)
 * -----------------------------------
 * {
 *      linkAction,
 *      startPoint,
 *      endPoint,
 *      linkedPublicKey,
 * 
 *      maxFee //(only if not multisig)
 * }
 */
static int parse_voting_key_link_txn_content( buffer_t* rawTxData, fields_array_t* fields )
{
    // get header
    const voting_key_link_header_t* txn = (const voting_key_link_header_t*) buffer_offset_ptr_and_seek( rawTxData, sizeof(voting_key_link_header_t)); // Read data and security check
    if( !txn ) { return E_NOT_ENOUGH_DATA; }
    
    // create fields from extracted data
    BAIL_IF( add_new_field(fields, XYM_UINT8_KL_TYPE,             STI_UINT8,      sizeof(uint8_t),       (const uint8_t*) &txn->linkAction)      ); // add link action type
    BAIL_IF( add_new_field(fields, XYM_UINT32_VKL_START_POINT,    STI_UINT32,     sizeof(uint32_t),      (const uint8_t*) &txn->startPoint)      ); // add start point
    BAIL_IF( add_new_field(fields, XYM_UINT32_VKL_END_POINT,      STI_UINT32,     sizeof(uint32_t),      (const uint8_t*) &txn->endPoint)        ); // add stop point
    BAIL_IF( add_new_field(fields, XYM_PUBLICKEY_VOTING_KEY_LINK, STI_PUBLIC_KEY, XYM_PUBLIC_KEY_LENGTH, (const uint8_t*) &txn->linkedPublicKey) ); // add linked public key
    
    return E_SUCCESS;
}




/**
 * HashLockTransaction (alias: LockFundsTransaction)
 * https://docs.symbolplatform.com/serialization/lock_hash.html#hashlocktransaction
 * 
 * 
 * transaction data (rawTxData)
 * ----------------------------------
 * {
 *      uint64_t maxFee;    ///< (only if not multisig)
 *      uint64_t deadline;  ///< (only if not multisig)
 * 
 *      uint64_t mosaicId;
 *      uint64_t amount;
 *      uint64_t blockDuration;
 *      uint8_t  aggregateBondedHash[XYM_TRANSACTION_HASH_LENGTH];
 * }
 * 
 * Output (fields)
 * -----------------------------------
 * {
 *      mosaicId+amount,
 *      blockDuration,
 *      aggregateBondedHash,
 * 
 *      maxFee //(only if not multisig)
 * }
 */
static int parse_fund_lock_txn_content( buffer_t* rawTxData, fields_array_t* fields )
{
    // get header
    const fl_header_t *txn = (const fl_header_t*) buffer_offset_ptr_and_seek( rawTxData, sizeof(fl_header_t)); // Read data and security check
    if( !txn ) { return E_NOT_ENOUGH_DATA; }
        
    // add fields    
    BAIL_IF( add_new_field(fields, XYM_MOSAIC_HL_QUANTITY, STI_MOSAIC_CURRENCY, sizeof(mosaic_t),            (const uint8_t*) &txn->mosaic)              ); // Show lock quantity
    BAIL_IF( add_new_field(fields, XYM_UINT64_DURATION,    STI_UINT64,          sizeof(uint64_t),            (const uint8_t*) &txn->blockDuration)       ); // Show duration
    BAIL_IF( add_new_field(fields, XYM_HASH256_HL_HASH,    STI_HASH256,         XYM_TRANSACTION_HASH_LENGTH, (const uint8_t*) &txn->aggregateBondedHash) ); // Show transaction hash
    
    return E_SUCCESS;
}


static int parseWithFee( buffer_t* rawTxData, fields_array_t* fields, int (*parser)(buffer_t* b, fields_array_t* f) )
{
    // get fee
    const txn_fee_t *fee = (const txn_fee_t*) buffer_offset_ptr_and_seek( rawTxData, sizeof(txn_fee_t)); // get fee
    if( !fee ) { return E_NOT_ENOUGH_DATA; }


    // call main parser
    int status = parser( rawTxData, fields );
    if( status != E_SUCCESS) { return status; }

    // Show tx fee
    BAIL_IF(add_new_field(fields, XYM_UINT64_TXN_FEE, STI_XYM, sizeof(uint64_t), (const uint8_t*) &fee->maxFee));
        
    return E_SUCCESS;
}


static int parse_inner_txn_content( buffer_t* rawTxData, uint32_t len, bool isCosigning, fields_array_t* fields ) 
{
    uint32_t totalSize = 0;

    do 
    {
        // get header
        const inner_tx_header_t *txn = (const inner_tx_header_t*) buffer_offset_ptr_and_seek( rawTxData, sizeof(inner_tx_header_t) ); // Read data and security check
        if( !txn ) { return E_NOT_ENOUGH_DATA; }

        totalSize += txn->size;
        
        // Show Transaction type
        BAIL_IF(add_new_field(fields, isCosigning ? XYM_UINT16_TRANSACTION_DETAIL_TYPE : XYM_UINT16_INNER_TRANSACTION_TYPE, STI_UINT16, sizeof(uint16_t), (const uint8_t*) &txn->innerTxType));
        switch( txn->innerTxType )
        {
            case XYM_TXN_TRANSFER:                      { BAIL_IF( parse_transfer_txn_content                      (rawTxData, fields) ); break; }
            case XYM_TXN_MOSAIC_DEFINITION:             { BAIL_IF( parse_mosaic_definition_txn_content             (rawTxData, fields) ); break; }
            case XYM_TXN_MOSAIC_SUPPLY_CHANGE:          { BAIL_IF( parse_mosaic_supply_change_txn_content          (rawTxData, fields) ); break; }
            case XYM_TXN_MODIFY_MULTISIG_ACCOUNT:       { BAIL_IF( parse_multisig_account_modification_txn_content (rawTxData, fields) ); break; } 
            case XYM_TXN_REGISTER_NAMESPACE:            { BAIL_IF( parse_namespace_registration_txn_content        (rawTxData, fields) ); break; }
            case XYM_TXN_ACCOUNT_METADATA:              { BAIL_IF( parse_account_metadata_txn_content              (rawTxData, fields) ); break; }
            case XYM_TXN_MOSAIC_METADATA:               { BAIL_IF( parse_mosaic_metadata_txn_content               (rawTxData, fields) ); break; }
            case XYM_TXN_NAMESPACE_METADATA:            { BAIL_IF( parse_namespace_metadata_txn_content            (rawTxData, fields) ); break; }
                                                                                                                                     
            case XYM_TXN_ADDRESS_ALIAS:                 { BAIL_IF( parse_address_alias_txn_content                 (rawTxData, fields) ); break; }
            case XYM_TXN_MOSAIC_ALIAS:                  { BAIL_IF( parse_mosaic_alias_txn_content                  (rawTxData, fields) ); break; }
                                                                                                                                     
            case XYM_TXN_ACCOUNT_ADDRESS_RESTRICTION:   { BAIL_IF( parse_account_address_restriction_txn_content   (rawTxData, fields) ); break; }
            case XYM_TXN_ACCOUNT_MOSAIC_RESTRICTION:    { BAIL_IF( parse_account_mosaic_restriction_txn_content    (rawTxData, fields) ); break; }
            case XYM_TXN_ACCOUNT_OPERATION_RESTRICTION: { BAIL_IF( parse_account_operation_restriction_txn_content (rawTxData, fields) ); break; }
                                                                                                                                     
            case XYM_TXN_ACCOUNT_KEY_LINK:              { BAIL_IF( parse_account_key_link_txn_content              (rawTxData, fields) ); break; }
            case XYM_TXN_NODE_KEY_LINK:                 { BAIL_IF( parse_node_key_link_txn_content                 (rawTxData, fields) ); break; }
            case XYM_TXN_VRF_KEY_LINK:                  { BAIL_IF( parse_vrf_key_link_txn_content                  (rawTxData, fields) ); break; }
                                                                                                                                     
            case XYM_TXN_VOTING_KEY_LINK:               { BAIL_IF( parse_voting_key_link_txn_content               (rawTxData, fields) ); break; }              
            case XYM_TXN_FUND_LOCK:                     { BAIL_IF( parse_fund_lock_txn_content                     (rawTxData, fields) ); break; }
              
            default:
            {
              return E_INVALID_DATA;
            }
        }
        
        // fill zeros
        bool succ = buffer_seek( rawTxData, txn->size % ALIGNMENT_BYTES == 0 ? 0 : ALIGNMENT_BYTES - (txn->size % ALIGNMENT_BYTES));
        if( !succ ) { return E_INVALID_DATA; }

    } while (totalSize < len - sizeof(inner_tx_header_t));

    
    return E_SUCCESS;
}


static int parse_aggregate_txn_content( buffer_t* rawTxData, fields_array_t* fields )
{
    // get aggregate header
    const aggregate_txn_t *txn = (const aggregate_txn_t*) buffer_offset_ptr_and_seek(rawTxData, sizeof(aggregate_txn_t));
    if( !txn ) { return E_NOT_ENOUGH_DATA; }

    bool isCosigning = (transactionContext.rawTxLength == XYM_TRANSACTION_HASH_LENGTH);
    const uint8_t* p_tx_hash = isCosigning ? rawTxData->ptr : txn->transactionHash;
    
    // add fields
    BAIL_IF( add_new_field(fields, XYM_HASH256_AGG_HASH, STI_HASH256, XYM_TRANSACTION_HASH_LENGTH, p_tx_hash) ); // add transaction hash
    if( !buffer_can_read(rawTxData, txn->payloadSize) ) { return E_INVALID_DATA; }
    BAIL_IF( parse_inner_txn_content(rawTxData, txn->payloadSize, isCosigning, fields) );

    return E_SUCCESS;
}



static int parse_txn_detail( buffer_t *rawTxData, const common_header_t *txn, fields_array_t* fields )
{
    int result;
    fields->numFields = 0;

    // Show Transaction type
    BAIL_IF( add_new_field(fields, XYM_UINT16_TRANSACTION_TYPE, STI_UINT16, sizeof(uint16_t), (const uint8_t*) &txn->transactionType) );

    switch( txn->transactionType )
    {
        case XYM_TXN_TRANSFER:                     { result = parseWithFee( rawTxData, fields, parse_transfer_txn_content                      ); break; }
        case XYM_TXN_AGGREGATE_COMPLETE:           { result = parseWithFee( rawTxData, fields, parse_aggregate_txn_content                     ); break; }
        case XYM_TXN_AGGREGATE_BONDED:             { result = parseWithFee( rawTxData, fields, parse_aggregate_txn_content                     ); break; }
        case XYM_TXN_MODIFY_MULTISIG_ACCOUNT:      { result = parseWithFee( rawTxData, fields, parse_multisig_account_modification_txn_content ); break; }
        case XYM_TXN_REGISTER_NAMESPACE:           { result = parseWithFee( rawTxData, fields, parse_namespace_registration_txn_content        ); break; }
        case XYM_TXN_ADDRESS_ALIAS:                { result = parseWithFee( rawTxData, fields, parse_address_alias_txn_content                 ); break; }
        case XYM_TXN_MOSAIC_ALIAS:                 { result = parseWithFee( rawTxData, fields, parse_mosaic_alias_txn_content                  ); break; }

        case XYM_TXN_ACCOUNT_ADDRESS_RESTRICTION:  { result = parseWithFee( rawTxData, fields, parse_account_address_restriction_txn_content   ); break; }
        case XYM_TXN_ACCOUNT_MOSAIC_RESTRICTION:   { result = parseWithFee( rawTxData, fields, parse_account_mosaic_restriction_txn_content    ); break; }
        case XYM_TXN_ACCOUNT_OPERATION_RESTRICTION:{ result = parseWithFee( rawTxData, fields, parse_account_operation_restriction_txn_content ); break; }

        case XYM_TXN_ACCOUNT_KEY_LINK:             { result = parseWithFee( rawTxData, fields, parse_account_key_link_txn_content              ); break; }
        case XYM_TXN_NODE_KEY_LINK:                { result = parseWithFee( rawTxData, fields, parse_node_key_link_txn_content                 ); break; }
        case XYM_TXN_VRF_KEY_LINK:                 { result = parseWithFee( rawTxData, fields, parse_vrf_key_link_txn_content                  ); break; }

        case XYM_TXN_VOTING_KEY_LINK:              { result = parseWithFee( rawTxData, fields, parse_voting_key_link_txn_content               ); break; }
        case XYM_TXN_MOSAIC_DEFINITION:            { result = parseWithFee( rawTxData, fields, parse_mosaic_definition_txn_content             ); break; }
        case XYM_TXN_MOSAIC_SUPPLY_CHANGE:         { result = parseWithFee( rawTxData, fields, parse_mosaic_supply_change_txn_content          ); break; }
        case XYM_TXN_FUND_LOCK:                    { result = parseWithFee( rawTxData, fields, parse_fund_lock_txn_content                     ); break; }
          
        default:
        {
            result = E_INVALID_DATA;
            break;
        }
    }
    
    return result;
}

static void set_sign_data_length( const buffer_t* rawTxdata, uint16_t transactionType ) //TODO: dont change global transactionContext here!!
{
    if( (transactionType == XYM_TXN_AGGREGATE_COMPLETE) || (transactionType == XYM_TXN_AGGREGATE_BONDED) )
    {
        const unsigned char TESTNET_GENERATION_HASH[] = { 0x49, 0xD6, 0xE1, 0xCE, 0x27, 0x6A, 0x85, 0xB7,
                                                          0x0E, 0xAF, 0xE5, 0x23, 0x49, 0xAA, 0xCC, 0xA3,
                                                          0x89, 0x30, 0x2E, 0x7A, 0x97, 0x54, 0xBC, 0xF1,
                                                          0x22, 0x1E, 0x79, 0x49, 0x4F, 0xC6, 0x65, 0xA4 };

        const unsigned char MAINNET_GENERATION_HASH[] = { 0x57, 0xF7, 0xDA, 0x20, 0x50, 0x08, 0x02, 0x6C,
                                                          0x77, 0x6C, 0xB6, 0xAE, 0xD8, 0x43, 0x39, 0x3F,
                                                          0x04, 0xCD, 0x45, 0x8E, 0x0A, 0xA2, 0xD9, 0xF1,
                                                          0xD5, 0xF3, 0x1A, 0x40, 0x20, 0x72, 0xB2, 0xD6 };

        const bool           is_using_mainnet = (transactionContext.bip32Path[1] & 0x7FFFFFFF) == 4343; // checks if the coin_type field of bip32 path is 'symbol'
        const unsigned char* net_hash         = is_using_mainnet ? MAINNET_GENERATION_HASH : TESTNET_GENERATION_HASH;
        const bool           hashes_equal     = memcmp(net_hash, rawTxdata->ptr, XYM_TRANSACTION_HASH_LENGTH) == 0;

        if( hashes_equal )
        {
            // Sign data from generation hash to transaction hash
            // XYM_AGGREGATE_SIGNING_LENGTH = XYM_TRANSACTION_HASH_LENGTH
            //                                + sizeof(common_header_t) + sizeof(txn_fee_t) = 84
            transactionContext.rawTxLength = XYM_AGGREGATE_SIGNING_LENGTH;
        }
        else 
        {
            // Sign transaction hash only (multisig cosigning transaction)
            transactionContext.rawTxLength = XYM_TRANSACTION_HASH_LENGTH;
        }
    }
    else 
    {
        // Sign all data in the transaction
        transactionContext.rawTxLength = rawTxdata->size;
    }
}


int parse_txn_context( buffer_t* rawTxdata, fields_array_t* fields )
{
    // get common header
    const common_header_t* txnHeader = (const common_header_t*) buffer_offset_ptr( rawTxdata );
    
    // move buffer offset to next data
    const bool succ = buffer_seek( rawTxdata, sizeof(common_header_t) );
    if( !succ ) { return E_NOT_ENOUGH_DATA; }

    set_sign_data_length( rawTxdata, txnHeader->transactionType );
    return parse_txn_detail( rawTxdata, txnHeader, fields );
}
