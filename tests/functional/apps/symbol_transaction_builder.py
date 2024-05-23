# Transaction builder for Ledger Symbol application
# The input, a JSON format is a custom one.
# Examples can be found in the tests/corpus directory.
# Maybe its readability could by improved by introducing more enums for network values, payload types, etc..

from struct import pack

INNER_TX_HEADER_SIZE = 4 + 4 + 32 + 4 + 1 + 1 + 2
ALIGNMENT_BYTES = 8

TRANSACTION_TYPES = {
    'TRANSFER': 0x4154,
    'REGISTER_NAMESPACE': 0x414E,
    'ADDRESS_ALIAS': 0x424E,
    'MOSAIC_ALIAS': 0x434E,
    'MOSAIC_DEFINITION': 0x414D,
    'MOSAIC_SUPPLY_CHANGE': 0x424D,
    'MODIFY_MULTISIG_ACCOUNT': 0x4155,
    'AGGREGATE_COMPLETE': 0x4141,
    'AGGREGATE_BONDED': 0x4241,
    'ACCOUNT_METADATA': 0x4144,
    'MOSAIC_METADATA': 0x4244,
    'NAMESPACE_METADATA': 0x4344,
    'ACCOUNT_ADDRESS_RESTRICTION': 0x4150,
    'ACCOUNT_MOSAIC_RESTRICTION': 0x4250,
    'ACCOUNT_OPERATION_RESTRICTION': 0x4350,
    'MOSAIC_ADDRESS_RESTRICTION': 0x4251,
    'MOSAIC_GLOBAL_RESTRICTION': 0x4151,
    'ACCOUNT_KEY_LINK': 0x414C,
    'NODE_KEY_LINK': 0x424C,
    'VOTING_KEY_LINK': 0x4143,
    'VRF_KEY_LINK': 0x4243,
    'FUND_LOCK': 0x4148,
    'SECRET_LOCK': 0x4152,
    'SECRET_PROOF': 0x4252,
}


def write_int8_t(value):
    return pack('<b', value)


def write_uint8_t(value):
    return pack('<B', value)


def write_int16_t(value):
    return pack('<h', value)


def write_uint16_t(value):
    return pack('<H', value)


def write_uint32_t(value):
    return pack('<I', value)


def write_uint64_t(value):
    return pack('<Q', value)


def write_len_prefixed_string(string):
    data = write_uint8_t(len(string))
    data += string.encode('utf-8')
    return data


def write_address(address):
    data = bytes.fromhex(address)
    return data


def write_public_key(public_key):
    data = bytes.fromhex(public_key)
    return data


def encode_common_txn_header(header):
    data = bytes.fromhex(header['transactionHash'])
    data += write_uint8_t(header['version'])
    data += write_uint8_t(header['networkType'])
    data += write_uint16_t(TRANSACTION_TYPES[header['transactionType']])
    data += write_uint64_t(header['maxFee'])
    data += write_uint64_t(header['deadline'])
    return data, header['transactionType']


def encode_transfer_txn_content(fields):
    mosaic_list = fields.get('mosaicList', [])
    message = fields.get('message', None)
    if message:
        message = bytes.fromhex(message)
        messageSize = 1 + len(message)
    else:
        messageSize = 0

    data = write_address(fields['recipient'])
    data += write_uint16_t(messageSize)
    data += write_uint8_t(len(mosaic_list))
    data += write_uint32_t(0)  # reserved
    data += write_uint8_t(0)  # reserved

    for mosaic in mosaic_list:
        data += write_uint64_t(mosaic['mosaicId'])
        data += write_uint64_t(mosaic['amount'])

    if message:
        data += write_uint8_t(fields['messageType'])
        data += message

    return data


def encode_mosaic_definition_txn_content(fields):
    data = write_uint64_t(fields['mosaicId'])
    data += write_uint64_t(fields['duration'])
    data += write_uint32_t(fields['nonce'])
    data += write_uint8_t(fields['flag'])
    data += write_uint8_t(fields['divisibility'])
    return data


def encode_mosaic_supply_change_txn_content(fields):
    data = write_uint64_t(fields['mosaicId'])
    data += write_uint64_t(fields['amount'])
    data += write_uint8_t(fields['action'])
    return data


def encode_multisig_account_modification_txn_content(fields):
    data = write_int8_t(fields['minRemovalDelta'])
    data += write_int8_t(fields['minApprovalDelta'])
    data += write_uint8_t(len(fields['addressAdditions']))
    data += write_uint8_t(len(fields['addressDeletions']))
    data += write_uint32_t(0)  # reserved
    for address in fields['addressAdditions']:
        data += write_address(address)
    for address in fields['addressDeletions']:
        data += write_address(address)
    return data


def encode_namespace_registration_txn_content(fields):
    data = write_uint64_t(fields['duration'])
    data += write_uint64_t(fields['namespaceId'])
    data += write_uint8_t(fields['registrationType'])
    data += write_len_prefixed_string(fields['namespaceName'])
    return data


def encode_account_metadata_txn_content(fields):
    data = write_address(fields['address'])
    data += write_uint64_t(fields['metadataKey'])
    data += write_int16_t(fields['valueSizeDelta'])
    value = bytes.fromhex(fields['value'])
    data += write_uint16_t(len(value))
    data += value
    return data


def encode_metadata_txn_content(fields):
    data = write_address(fields['address'])
    data += write_uint64_t(fields['metadataKey'])
    data += write_uint64_t(fields['mosaicNamespaceId'])
    data += write_int16_t(fields['valueSizeDelta'])
    value = bytes.fromhex(fields['value'])
    data += write_uint16_t(len(value))
    data += value
    return data


def encode_mosaic_metadata_txn_content(fields):
    return encode_metadata_txn_content(fields)


def encode_namespace_metadata_txn_content(fields):
    return encode_metadata_txn_content(fields)


def encode_address_alias_txn_content(fields):
    data = write_uint64_t(fields['namespaceId'])
    data += write_address(fields['address'])
    data += write_uint8_t(fields['aliasAction'])
    return data


def encode_mosaic_alias_txn_content(fields):
    data = write_uint64_t(fields['namespaceId'])
    data += write_uint64_t(fields['mosaicId'])
    data += write_uint8_t(fields['aliasAction'])
    return data


def encode_account_address_restriction_txn_content(fields):
    data = write_uint16_t(fields['restrictionFlags'])
    data += write_uint8_t(len(fields['restrictionAdditions']))
    data += write_uint8_t(len(fields['restrictionDeletions']))
    data += write_uint32_t(0)  # reserved
    for address in fields['restrictionAdditions']:
        data += write_address(address)
    for address in fields['restrictionDeletions']:
        data += write_address(address)
    return data


def encode_account_mosaic_restriction_txn_content(fields):
    data = write_uint16_t(fields['restrictionFlags'])
    data += write_uint8_t(len(fields['restrictionAdditions']))
    data += write_uint8_t(len(fields['restrictionDeletions']))
    data += write_uint32_t(0)  # reserved
    for mosaicId in fields['restrictionAdditions']:
        data += write_uint64_t(mosaicId)
    for mosaicId in fields['restrictionDeletions']:
        data += write_uint64_t(mosaicId)
    return data


def encode_account_operation_restriction_txn_content(fields):
    data = write_uint16_t(fields['restrictionFlags'])
    data += write_uint8_t(len(fields['restrictionAdditions']))
    data += write_uint8_t(len(fields['restrictionDeletions']))
    data += write_uint32_t(0)  # reserved
    for operation in fields['restrictionAdditions']:
        data += write_uint16_t(operation)
    for operation in fields['restrictionDeletions']:
        data += write_uint16_t(operation)
    return data


def encode_key_link_txn_content(fields):
    data = write_public_key(fields['linkedPublicKey'])
    data += write_uint8_t(fields['linkAction'])
    return data


def encode_account_key_link_txn_content(fields):
    return encode_key_link_txn_content(fields)


def encode_node_key_link_txn_content(fields):
    return encode_key_link_txn_content(fields)


def encode_vrf_key_link_txn_content(fields):
    return encode_key_link_txn_content(fields)


def encode_voting_key_link_txn_content(fields):
    data = write_public_key(fields['linkedPublicKey'])
    data += write_uint32_t(fields['startPoint'])
    data += write_uint32_t(fields['endPoint'])
    data += write_uint8_t(fields['linkAction'])
    return data


def encode_fund_lock_txn_content(fields):
    data = write_uint64_t(fields['mosaicId'])
    data += write_uint64_t(fields['amount'])
    data += write_uint64_t(fields['blockDuration'])
    data += bytes.fromhex(fields['aggregateBondedHash'])
    return data


def encode_inner_tx_header(header, size):
    data = write_uint32_t(size)
    data += write_uint32_t(0)  # reserved
    data += write_public_key(header['signerPublicKey'])
    data += write_uint32_t(0)  # reserved
    data += write_uint8_t(header['version'])
    data += write_uint8_t(header['networkType'])
    data += write_uint16_t(TRANSACTION_TYPES[header['transactionType']])
    return data


def encode_aggregate_txn_content(fields):
    data = bytes.fromhex(fields['transactionHash'])

    payload_data = b""
    for transaction in fields['transactions']:
        transaction_type = transaction['inner_tx_header']['transactionType']
        fields_data = encode_txn_detail(transaction_type, transaction['fields'])
        size = len(fields_data) + INNER_TX_HEADER_SIZE
        header_data = encode_inner_tx_header(transaction['inner_tx_header'], size)
        payload_data += header_data
        payload_data += fields_data

        # Handle alignment
        alignement_size = size % ALIGNMENT_BYTES
        if alignement_size:
            payload_data += bytes.fromhex('00') * (ALIGNMENT_BYTES - alignement_size)

    data += write_uint32_t(len(payload_data))
    data += write_uint32_t(0)  # reserved
    data += payload_data
    return data


def encode_txn_detail(transaction_type, fields):
    if transaction_type == 'TRANSFER':
        return encode_transfer_txn_content(fields)
    elif transaction_type == 'AGGREGATE_COMPLETE':
        return encode_aggregate_txn_content(fields)
    elif transaction_type == 'AGGREGATE_BONDED':
        return encode_aggregate_txn_content(fields)
    elif transaction_type == 'MODIFY_MULTISIG_ACCOUNT':
        return encode_multisig_account_modification_txn_content(fields)
    elif transaction_type == 'REGISTER_NAMESPACE':
        return encode_namespace_registration_txn_content(fields)
    elif transaction_type == 'ADDRESS_ALIAS':
        return encode_address_alias_txn_content(fields)
    elif transaction_type == 'MOSAIC_ALIAS':
        return encode_mosaic_alias_txn_content(fields)
    elif transaction_type == 'ACCOUNT_ADDRESS_RESTRICTION':
        return encode_account_address_restriction_txn_content(fields)
    elif transaction_type == 'ACCOUNT_MOSAIC_RESTRICTION':
        return encode_account_mosaic_restriction_txn_content(fields)
    elif transaction_type == 'ACCOUNT_OPERATION_RESTRICTION':
        return encode_account_operation_restriction_txn_content(fields)
    elif transaction_type == 'ACCOUNT_KEY_LINK':
        return encode_account_key_link_txn_content(fields)
    elif transaction_type == 'NODE_KEY_LINK':
        return encode_node_key_link_txn_content(fields)
    elif transaction_type == 'VRF_KEY_LINK':
        return encode_vrf_key_link_txn_content(fields)
    elif transaction_type == 'VOTING_KEY_LINK':
        return encode_voting_key_link_txn_content(fields)
    elif transaction_type == 'MOSAIC_DEFINITION':
        return encode_mosaic_definition_txn_content(fields)
    elif transaction_type == 'MOSAIC_SUPPLY_CHANGE':
        return encode_mosaic_supply_change_txn_content(fields)
    elif transaction_type == 'FUND_LOCK':
        return encode_fund_lock_txn_content(fields)
    elif transaction_type == 'ACCOUNT_METADATA':
        return encode_account_metadata_txn_content(fields)
    elif transaction_type == 'NAMESPACE_METADATA':
        return encode_namespace_metadata_txn_content(fields)
    elif transaction_type == 'MOSAIC_METADATA':
        return encode_mosaic_metadata_txn_content(fields)
    assert False


def encode_txn_context(transaction):
    data, transaction_type = encode_common_txn_header(transaction['common_txn_header'])
    data += encode_txn_detail(transaction_type, transaction['fields'])
    return data
