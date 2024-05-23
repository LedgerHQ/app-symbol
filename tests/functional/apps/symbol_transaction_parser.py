from struct import unpack

INNER_TX_HEADER_SIZE = 4 + 4 + 32 + 4 + 1 + 1 + 2
ALIGNMENT_BYTES = 8

TRANSACTION_TYPES = {
    0x4154: 'TRANSFER',
    0x414E: 'REGISTER_NAMESPACE',
    0x424E: 'ADDRESS_ALIAS',
    0x434E: 'MOSAIC_ALIAS',
    0x414D: 'MOSAIC_DEFINITION',
    0x424D: 'MOSAIC_SUPPLY_CHANGE',
    0x4155: 'MODIFY_MULTISIG_ACCOUNT',
    0x4141: 'AGGREGATE_COMPLETE',
    0x4241: 'AGGREGATE_BONDED',
    0x4144: 'ACCOUNT_METADATA',
    0x4244: 'MOSAIC_METADATA',
    0x4344: 'NAMESPACE_METADATA',
    0x4150: 'ACCOUNT_ADDRESS_RESTRICTION',
    0x4250: 'ACCOUNT_MOSAIC_RESTRICTION',
    0x4350: 'ACCOUNT_OPERATION_RESTRICTION',
    0x4251: 'MOSAIC_ADDRESS_RESTRICTION',
    0x4151: 'MOSAIC_GLOBAL_RESTRICTION',
    0x414C: 'ACCOUNT_KEY_LINK',
    0x424C: 'NODE_KEY_LINK',
    0x4143: 'VOTING_KEY_LINK',
    0x4243: 'VRF_KEY_LINK',
    0x4148: 'FUND_LOCK',
    0x4152: 'SECRET_LOCK',
    0x4252: 'SECRET_PROOF',
}


def read_int8_t(buffer):
    return buffer[1:], unpack('<b', buffer[:1])[0]


def read_uint8_t(buffer):
    return buffer[1:], unpack('<B', buffer[:1])[0]


def read_int16_t(buffer):
    return buffer[2:], unpack('<h', buffer[:2])[0]


def read_uint16_t(buffer):
    return buffer[2:], unpack('<H', buffer[:2])[0]


def read_uint32_t(buffer):
    return buffer[4:], unpack('<I', buffer[:4])[0]


def read_uint64_t(buffer):
    return buffer[8:], unpack('<Q', buffer[:8])[0]


def read_array_data(buffer, length):
    data = buffer[:length]
    assert len(data) == length
    return buffer[length:], data


def read_len_prefixed_data(buffer):
    buffer, length = read_uint8_t(buffer)
    return read_array_data(buffer, length)


def read_len_prefixed_string(buffer):
    buffer, data = read_len_prefixed_data(buffer)
    return buffer, data.decode('utf-8')


def read_address(buffer):
    buffer, array = read_array_data(buffer, 24)
    return buffer, array.hex()


def read_public_key(buffer):
    buffer, public_key = read_array_data(buffer, 32)
    return buffer, public_key.hex()


def decode_common_txn_header(buffer):
    buffer, transactionHash = read_array_data(buffer, 32)
    buffer, version = read_uint8_t(buffer)
    buffer, networkType = read_uint8_t(buffer)
    buffer, value = read_uint16_t(buffer)
    transactionType = TRANSACTION_TYPES[value]
    buffer, maxFee = read_uint64_t(buffer)
    buffer, deadline = read_uint64_t(buffer)

    data = {
        "transactionHash": transactionHash.hex(),
        "version": version,
        "networkType": networkType,
        "transactionType": transactionType,
        "maxFee": maxFee,
        "deadline": deadline
    }
    return buffer, data


def decode_transfer_txn_content(buffer):
    buffer, recipient = read_address(buffer)
    buffer, messageSize = read_uint16_t(buffer)
    buffer, mosaicsNb = read_uint8_t(buffer)
    buffer, reserved = read_uint32_t(buffer)
    buffer, reserved = read_uint8_t(buffer)

    data = {
        "recipient": recipient
    }

    mosaicList = []
    for _ in range(mosaicsNb):
        buffer, mosaicId = read_uint64_t(buffer)
        buffer, amount = read_uint64_t(buffer)
        mosaicList.append({
            "mosaicId": mosaicId,
            "amount": amount,
        })

    data["mosaicList"] = mosaicList

    if messageSize:
        buffer, messageType = read_uint8_t(buffer)
        buffer, message = read_array_data(buffer, messageSize - 1)

        data["messageType"] = messageType
        data["message"] = message.hex()

    return buffer, data


def decode_mosaic_definition_txn_content(buffer):
    buffer, mosaicId = read_uint64_t(buffer)
    buffer, duration = read_uint64_t(buffer)
    buffer, nonce = read_uint32_t(buffer)
    buffer, flag = read_uint8_t(buffer)
    buffer, divisibility = read_uint8_t(buffer)

    data = {
        "mosaicId": mosaicId,
        "duration": duration,
        "nonce": nonce,
        "flag": flag,
        "divisibility": divisibility
    }

    return buffer, data


def decode_mosaic_supply_change_txn_content(buffer):
    buffer, mosaicId = read_uint64_t(buffer)
    buffer, amount = read_uint64_t(buffer)
    buffer, action = read_uint8_t(buffer)

    data = {
        "mosaicId": mosaicId,
        "amount": amount,
        "action": action
    }

    return buffer, data


def decode_multisig_account_modification_txn_content(buffer):
    buffer, minRemovalDelta = read_int8_t(buffer)
    buffer, minApprovalDelta = read_int8_t(buffer)
    buffer, addressAdditionsNb = read_uint8_t(buffer)
    buffer, addressDeletionsNb = read_uint8_t(buffer)
    buffer, reserved = read_uint32_t(buffer)

    addressAdditions = []
    for _ in range(addressAdditionsNb):
        buffer, address = read_address(buffer)
        addressAdditions.append(address)

    addressDeletions = []
    for _ in range(addressDeletionsNb):
        buffer, address = read_address(buffer)
        addressDeletions.append(address)

    data = {
        "minRemovalDelta": minRemovalDelta,
        "minApprovalDelta": minApprovalDelta,
        "addressAdditions": addressAdditions,
        "addressDeletions": addressDeletions
    }

    return buffer, data


def decode_namespace_registration_txn_content(buffer):
    buffer, duration = read_uint64_t(buffer)
    buffer, namespaceId = read_uint64_t(buffer)
    buffer, registrationType = read_uint8_t(buffer)
    buffer, namespaceName = read_len_prefixed_string(buffer)

    data = {
        "duration": duration,
        "namespaceId": namespaceId,
        "registrationType": registrationType,
        "namespaceName": namespaceName
    }

    return buffer, data


def decode_account_metadata_txn_content(buffer):
    buffer, address = read_address(buffer)
    buffer, metadataKey = read_uint64_t(buffer)
    buffer, valueSizeDelta = read_int16_t(buffer)
    buffer, valueLen = read_uint16_t(buffer)
    buffer, value = read_array_data(buffer, valueLen)

    data = {
        "address": address,
        "metadataKey": metadataKey,
        "valueSizeDelta": valueSizeDelta,
        "value": value.hex()
    }

    return buffer, data


def decode_metadata_txn_content(buffer):
    buffer, address = read_address(buffer)
    buffer, metadataKey = read_uint64_t(buffer)
    buffer, mosaicNamespaceId = read_uint64_t(buffer)
    buffer, valueSizeDelta = read_int16_t(buffer)
    buffer, valueLen = read_uint16_t(buffer)
    buffer, value = read_array_data(buffer, valueLen)

    data = {
        "address": address,
        "metadataKey": metadataKey,
        "mosaicNamespaceId": mosaicNamespaceId,
        "valueSizeDelta": valueSizeDelta,
        "value": value.hex()
    }

    return buffer, data


def decode_mosaic_metadata_txn_content(buffer):
    return decode_metadata_txn_content(buffer)


def decode_namespace_metadata_txn_content(buffer):
    return decode_metadata_txn_content(buffer)


def decode_address_alias_txn_content(buffer):
    buffer, namespaceId = read_uint64_t(buffer)
    buffer, address = read_address(buffer)
    buffer, aliasAction = read_uint8_t(buffer)

    data = {
        "namespaceId": namespaceId,
        "address": address,
        "aliasAction": aliasAction
    }
    return buffer, data


def decode_mosaic_alias_txn_content(buffer):
    buffer, namespaceId = read_uint64_t(buffer)
    buffer, mosaicId = read_uint64_t(buffer)
    buffer, aliasAction = read_uint8_t(buffer)

    data = {
        "namespaceId": namespaceId,
        "mosaicId": mosaicId,
        "aliasAction": aliasAction
    }
    return buffer, data


def decode_account_address_restriction_txn_content(buffer):
    buffer, restrictionFlags = read_uint16_t(buffer)
    buffer, restrictionAdditionsNb = read_uint8_t(buffer)
    buffer, restrictionDeletionsNb = read_uint8_t(buffer)
    buffer, reserved = read_uint32_t(buffer)

    restrictionAdditions = []
    for _ in range(restrictionAdditionsNb):
        buffer, address = read_address(buffer)
        restrictionAdditions.append(address)

    restrictionDeletions = []
    for _ in range(restrictionDeletionsNb):
        buffer, address = read_address(buffer)
        restrictionDeletions.append(address)

    data = {
        "restrictionFlags": restrictionFlags,
        "restrictionAdditions": restrictionAdditions,
        "restrictionDeletions": restrictionDeletions
    }
    return buffer, data


def decode_account_mosaic_restriction_txn_content(buffer):
    buffer, restrictionFlags = read_uint16_t(buffer)
    buffer, restrictionAdditionsNb = read_uint8_t(buffer)
    buffer, restrictionDeletionsNb = read_uint8_t(buffer)
    buffer, reserved = read_uint32_t(buffer)

    restrictionAdditions = []
    for _ in range(restrictionAdditionsNb):
        buffer, mosaicId = read_uint64_t(buffer)
        restrictionAdditions.append(mosaicId)

    restrictionDeletions = []
    for _ in range(restrictionDeletionsNb):
        buffer, mosaicId = read_uint64_t(buffer)
        restrictionDeletions.append(mosaicId)

    data = {
        "restrictionFlags": restrictionFlags,
        "restrictionAdditions": restrictionAdditions,
        "restrictionDeletions": restrictionDeletions
    }
    return buffer, data


def decode_account_operation_restriction_txn_content(buffer):
    buffer, restrictionFlags = read_uint16_t(buffer)
    buffer, restrictionAdditionsNb = read_uint8_t(buffer)
    buffer, restrictionDeletionsNb = read_uint8_t(buffer)
    buffer, reserved = read_uint32_t(buffer)

    restrictionAdditions = []
    for _ in range(restrictionAdditionsNb):
        buffer, operation = read_uint16_t(buffer)
        restrictionAdditions.append(operation)

    restrictionDeletions = []
    for _ in range(restrictionDeletionsNb):
        buffer, operation = read_uint16_t(buffer)
        restrictionDeletions.append(operation)

    data = {
        "restrictionFlags": restrictionFlags,
        "restrictionAdditions": restrictionAdditions,
        "restrictionDeletions": restrictionDeletions
    }
    return buffer, data


def decode_key_link_txn_content(buffer):
    buffer, linkedPublicKey = read_public_key(buffer)
    buffer, linkAction = read_uint8_t(buffer)

    data = {
        "linkedPublicKey": linkedPublicKey,
        "linkAction": linkAction
    }
    return buffer, data


def decode_account_key_link_txn_content(buffer):
    return decode_key_link_txn_content(buffer)


def decode_node_key_link_txn_content(buffer):
    return decode_key_link_txn_content(buffer)


def decode_vrf_key_link_txn_content(buffer):
    return decode_key_link_txn_content(buffer)


def decode_voting_key_link_txn_content(buffer):
    buffer, linkedPublicKey = read_public_key(buffer)
    buffer, startPoint = read_uint32_t(buffer)
    buffer, endPoint = read_uint32_t(buffer)
    buffer, linkAction = read_uint8_t(buffer)

    data = {
        "linkedPublicKey": linkedPublicKey,
        "startPoint": startPoint,
        "endPoint": endPoint,
        "linkAction": linkAction
    }
    return buffer, data


def decode_fund_lock_txn_content(buffer):
    buffer, mosaicId = read_uint64_t(buffer)
    buffer, amount = read_uint64_t(buffer)
    buffer, blockDuration = read_uint64_t(buffer)
    buffer, aggregateBondedHash = read_array_data(buffer, 32)

    data = {
        "mosaicId": mosaicId,
        "amount": amount,
        "blockDuration": blockDuration,
        "aggregateBondedHash": aggregateBondedHash.hex()
    }
    return buffer, data


def decode_inner_tx_header(buffer):
    buffer, size = read_uint32_t(buffer)
    buffer, reserved = read_uint32_t(buffer)
    buffer, signerPublicKey = read_public_key(buffer)
    buffer, reserved = read_uint32_t(buffer)
    buffer, version = read_uint8_t(buffer)
    buffer, networkType = read_uint8_t(buffer)
    buffer, value = read_uint16_t(buffer)
    transactionType = TRANSACTION_TYPES[value]

    data = {
        "signerPublicKey": signerPublicKey,
        "version": version,
        "networkType": networkType,
        "transactionType": transactionType,
    }
    return buffer, data, size


def decode_aggregate_txn_content(buffer):
    buffer, transactionHash = read_array_data(buffer, 32)
    buffer, payload_data_len = read_uint32_t(buffer)
    buffer, reserved = read_uint32_t(buffer)

    transactions = []
    buffer, payload_data = read_array_data(buffer, payload_data_len)
    while payload_data:
        payload_data, header, size = decode_inner_tx_header(payload_data)
        payload_data, transaction_payload = read_array_data(payload_data, size - INNER_TX_HEADER_SIZE)

        transaction_payload, fields = decode_txn_detail(transaction_payload, header["transactionType"])

        # Handle alignment
        alignement_size = size % ALIGNMENT_BYTES
        if alignement_size:
            payload_data, _ = read_array_data(payload_data, ALIGNMENT_BYTES - alignement_size)

        transactions.append({
            'inner_tx_header': header,
            'fields': fields
        })

    data = {
        'transactionHash': transactionHash.hex(),
        'transactions': transactions
    }
    return buffer, data


def decode_txn_detail(buffer, transaction_type):
    if transaction_type == 'TRANSFER':
        return decode_transfer_txn_content(buffer)
    elif transaction_type == 'AGGREGATE_COMPLETE':
        return decode_aggregate_txn_content(buffer)
    elif transaction_type == 'AGGREGATE_BONDED':
        return decode_aggregate_txn_content(buffer)
    elif transaction_type == 'MODIFY_MULTISIG_ACCOUNT':
        return decode_multisig_account_modification_txn_content(buffer)
    elif transaction_type == 'REGISTER_NAMESPACE':
        return decode_namespace_registration_txn_content(buffer)
    elif transaction_type == 'ADDRESS_ALIAS':
        return decode_address_alias_txn_content(buffer)
    elif transaction_type == 'MOSAIC_ALIAS':
        return decode_mosaic_alias_txn_content(buffer)
    elif transaction_type == 'ACCOUNT_ADDRESS_RESTRICTION':
        return decode_account_address_restriction_txn_content(buffer)
    elif transaction_type == 'ACCOUNT_MOSAIC_RESTRICTION':
        return decode_account_mosaic_restriction_txn_content(buffer)
    elif transaction_type == 'ACCOUNT_OPERATION_RESTRICTION':
        return decode_account_operation_restriction_txn_content(buffer)
    elif transaction_type == 'ACCOUNT_KEY_LINK':
        return decode_account_key_link_txn_content(buffer)
    elif transaction_type == 'NODE_KEY_LINK':
        return decode_node_key_link_txn_content(buffer)
    elif transaction_type == 'VRF_KEY_LINK':
        return decode_vrf_key_link_txn_content(buffer)
    elif transaction_type == 'VOTING_KEY_LINK':
        return decode_voting_key_link_txn_content(buffer)
    elif transaction_type == 'MOSAIC_DEFINITION':
        return decode_mosaic_definition_txn_content(buffer)
    elif transaction_type == 'MOSAIC_SUPPLY_CHANGE':
        return decode_mosaic_supply_change_txn_content(buffer)
    elif transaction_type == 'FUND_LOCK':
        return decode_fund_lock_txn_content(buffer)
    elif transaction_type == 'ACCOUNT_METADATA':
        return decode_account_metadata_txn_content(buffer)
    elif transaction_type == 'NAMESPACE_METADATA':
        return decode_namespace_metadata_txn_content(buffer)
    elif transaction_type == 'MOSAIC_METADATA':
        return decode_mosaic_metadata_txn_content(buffer)
    assert False


def decode_txn_context(buffer):
    buffer, header = decode_common_txn_header(buffer)
    buffer, fields = decode_txn_detail(buffer, header["transactionType"])
    assert len(buffer) == 0
    return {'common_txn_header': header, 'fields': fields}
