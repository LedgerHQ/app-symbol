# Symbol's APDU package fields

## I. Shared parts

```console
01. CLA   (1 byte)
02. INS   (1 byte)
03. P1    (1 byte)
04. P2    (1 byte)
05. LC    (1 byte)
06. CDATA (1 byte)

07. Devariant path (44/4343/0/0/0) (20 bytes) (8000002C800010F7800000008000000080000000) (fixed)
08. Generation hash                (64 bytes) (1DFB2FAA9E7F054168B0C5FCB84F4DEB62CC2B4D317D861F3168D161F54EA78B) (fixed in all tx except cosignature tx)

09. Version                        (1 byte)   (01) (fixed)
10. Network Type                   (1 byte)   (98) (now fixed but can be change later)
11. Transaction Type               (2 bytes)  (depends on tx) (https://docs.symbolplatform.com/concepts/transaction.html#transaction-types)
12. Max Fee      (MOSAIC)          (8 bytes)  (depends on tx) (https://docs.symbolplatform.com/concepts/fees.html#fees)
13. Deadline      (MOSAIC)         (8 bytes)  (depends on tx) (Number of milliseconds elapsed since the creation of the nemesis block)
```

> *NOTE*: Fields with (MOSAIC) tag (field 12, 13) are NOT USED in the aggregate transactions

## II. Properties parts

### A. Normal tx

#### Transfer transaction schema

##### Transfer Tx

[Reference](ttps://docs.symbolplatform.com/serialization/transfer.html#transfertransaction)

```console
Property                                Types                                       Description
----------------------------------------------------------------------------------------------------------------------
recipientAddress                        24 Bytes                                      Transaction recipient.
messageSize                             uint16                                        Size of the attached message.
mosaicsCount                            uint8                                         Number of attached mosaics.
transferTransactionBody_Reserved1       uint32                                        Reserved padding to align mosaics on 8-byte boundary.
transferTransactionBody_Reserved2       uint8                                         Reserved padding to align mosaics on 8-byte boundary.
mosaics                                 array(UnresolvedMosaic(uint64), mosaicsCount) Attached mosaics to send.
message                                 array(byte, messageSize)                      Message type and hexadecimal payload.
```

Example:

Full raw transaction (ledger receive):

```console
E004008090058000002C800010F78000000080000000800000001DFB2FAA9E7F054168B0C5FCB84F4DEB62CC2B4D317D861F3168D161F54EA78B0198544180841E0000000000F6A98B390600000098F2A5E8E063AD1A9085EF5B5167E2F1A5645C48FA2C024917000100000000008ABEC5CA0D99625E40A5AE0200000000005468697320697320612074657374206D657373616765
```

##### Parsed above tx

```console
##### Shared parts

01 -> 06    E00400809005
07          8000002C800010F7800000008000000080000000
08          1DFB2FAA9E7F054168B0C5FCB84F4DEB62CC2B4D317D861F3168D161F54EA78B
09 -> 10    0198
11          5441
12          80841E0000000000
13          F6A98B3906000000

##### Properties parts
Address     98F2A5E8E063AD1A9085EF5B5167E2F1A5645C48FA2C0249
msgSize     1700
mosaicCount 01
Reserved1   00000000
Reserved2   00
ArrayMosaic 8ABEC5CA0D99625E 40A5AE0200000000
ArrayMsg    005468697320697320612074657374206D657373616765 (Hex to ascii --- This is a test message)
```

#### Namespace Registration Transaction schema

##### Namespace Registration Transaction

[Reference](https://docs.symbolplatform.com/serialization/namespace.html#namespaceregistrationtransaction)

```console
Property                                Types                                       Description
----------------------------------------------------------------------------------------------------------------------
duration                                 BlockDuration (uint64)                       Number of confirmed blocks you would like to rent. Required for root namespaces. (Optional)
parentId                                NamespaceId (uint64)                        Parent namespace identifier. Required for subnamespaces. (Optional)
id                                         NamespaceId (uint64)                        Namespace identifier.
registrationType                         uint8_t                                      Namespace registration type.
nameSize                                 uint8_t                                      Namespace name size in bytes.
name                                     array(bytes, nameSize)                         Namespace name.


(Optional): One package just has field duration or parentId at a time. Depends on registrationType
```

Example:

Full raw transaction (ledger receive):

```console
E00400806C058000002C800010F78000000080000000800000001DFB2FAA9E7F054168B0C5FCB84F4DEB62CC2B4D317D861F3168D161F54EA78B01984E4180841E00000000000985923906000000E803000000000000C880D8EBBA4A85A90011666F6F35373673676E6C78646E66626478
```

##### Parsed above tx

```console
##### Shared parts

01 -> 06    E00400806C05
07          8000002C800010F7800000008000000080000000
08          1DFB2FAA9E7F054168B0C5FCB84F4DEB62CC2B4D317D861F3168D161F54EA78B
09 -> 10    0198
11          4E41
12          80841E0000000000
13          0985923906000000

##### Properties parts
duration    E803000000000000
id          C880D8EBBA4A85A9
registrationType    00
nameSize    11
name        666F6F35373673676E6C78646E66626478
```

#### Mosaic Alias Transaction schema (0x434E)

##### Mosaic Alias Transaction

[Reference](https://docs.symbolplatform.com/serialization/namespace.html#mosaic-alias-transaction)

```console
Property                                Types                                       Description
----------------------------------------------------------------------------------------------------------------------
namespaceId                             NamespaceId (uint64)                         Identifier of the namespace that will become an alias.
mosaicId                                 MosaicId (uint64)                           Aliased mosaic identifier.
aliasAction                             AliasAction (uint8)                            Alias action.

```

Example:

Full raw transaction (ledger receive):

```console
E00400805A058000002C800010F78000000080000000800000001DFB2FAA9E7F054168B0C5FCB84F4DEB62CC2B4D317D861F3168D161F54EA78B01984E4380841E00000000009B5096390600000054C07E58ACD1A982CC403C7A113BDF7C00
```

##### Parsed above tx

```console
##### Shared parts

01 -> 06    E00400805A05
07          8000002C800010F7800000008000000080000000
08          1DFB2FAA9E7F054168B0C5FCB84F4DEB62CC2B4D317D861F3168D161F54EA78B
09 -> 10    0198
11          4E43
12          80841E0000000000
13          9B50963906000000

##### Properties parts
namespaceId 54C07E58ACD1A982
mosaicId    CC403C7A113BDF7C
aliasAction 00
```

#### Address Alias Transaction schema (0x424E)

##### Address Alias Transaction

[Reference](https://docs.symbolplatform.com/serialization/namespace.html#mosaic-alias-transaction)

```console
Property                                Types                                       Description
----------------------------------------------------------------------------------------------------------------------
namespaceId                             NamespaceId (uint64)                         Identifier of the namespace that will become an alias.
address                                 Address (24 bytes)                           Aliased address.
aliasAction                             AliasAction (uint8)                            Alias action.

```

Example:

Full raw transaction (ledger receive):

```console
E00400806A058000002C800010F78000000080000000800000001DFB2FAA9E7F054168B0C5FCB84F4DEB62CC2B4D317D861F3168D161F54EA78B01984E4280841E0000000000A92B97390600000054C07E58ACD1A98298F2A5E8E063AD1A9085EF5B5167E2F1A5645C48FA2C024901

```

##### Parsed above tx

```console
##### Shared parts

01 -> 06    E00400806A05
07          8000002C800010F7800000008000000080000000
08          1DFB2FAA9E7F054168B0C5FCB84F4DEB62CC2B4D317D861F3168D161F54EA78B
09 -> 10    0198
11          4E42
12          80841E0000000000
13          A92B973906000000

##### Properties parts
namespaceId 54C07E58ACD1A982
address     98F2A5E8E063AD1A9085EF5B5167E2F1A5645C48FA2C0249
aliasAction 01
```

#### Mosaic Supply Change Transaction schema (0x424E)

##### Mosaic Supply Change Transaction

[Reference](https://docs.symbolplatform.com/serialization/mosaic.html#mosaicsupplychangetransaction)

> *Note*: Available for inner tx transaction [Ref](#mosaic-supply-change-transaction-inner-tx-2)

```console
Property                                Types                                       Description
----------------------------------------------------------------------------------------------------------------------
mosaicId                                 UnresolvedMosaicId (uint64)                 Affected mosaic identifier.
delta                                     Amount (uint64)                             Amount of supply to increase or decrease.
action                                     MosaicSupplyChangeAction (uint8)            Supply change action.
```

Example:

Full raw transaction (ledger receive):

```console
E00400805A058000002C800010F78000000080000000800000001DFB2FAA9E7F054168B0C5FCB84F4DEB62CC2B4D317D861F3168D161F54EA78B01984D4280841E00000000001F2A933906000000CC403C7A113BDF7C40420F000000000001
```

##### Parsed above tx

```console
##### Shared parts

01 -> 06    E00400805A05
07          8000002C800010F7800000008000000080000000
08          1DFB2FAA9E7F054168B0C5FCB84F4DEB62CC2B4D317D861F3168D161F54EA78B
09 -> 10    0198
11          4D42
12          80841E0000000000
13          1F2A933906000000

##### Properties parts
mosaicId    CC403C7A113BDF7C
delta       40420F0000000000
action      01
```

#### Hash Lock Schemas schema

##### Hash Lock Schemas

[Reference](https://docs.symbolplatform.com/serialization/lock_hash.html#hashlocktransaction)

```console
Property                                Types                                       Description
----------------------------------------------------------------------------------------------------------------------
mosaic                                     UnresolvedMosaic     array(uint64,uint64)    Locked mosaic.
duration                                 BlockDuration     (uint64)                    Number of blocks for which a lock should be valid.
hash                                     Hash256 (32 bytes)                          AggregateBondedTransaction hash that has to be confirmed before unlocking the mosaics.
```

Example:

Full raw transaction (ledger receive):

```console
E004008081058000002C800010F78000000080000000000000001DFB2FAA9E7F054168B0C5FCB84F4DEB62CC2B4D317D861F3168D161F54EA78B0198484180841E0000000000D58B993906000000A84582052890A9518096980000000000E0010000000000002B51EBCBC3E40EFE8AF68A0408F5A72474B1327A64E3E3B47D9B139230C7833B
```

##### Parsed above tx

```console
##### Shared parts

01 -> 06    E00400808105
07          8000002C800010F78000000080000000000000000
08          1DFB2FAA9E7F054168B0C5FCB84F4DEB62CC2B4D317D861F3168D161F54EA78B
09 -> 10    0198
11          4841
12          80841E0000000000
13          D58B993906000000

##### Properties parts
mosaicId    A84582052890A951
amount      8096980000000000
duration    E001000000000000
hash        2B51EBCBC3E40EFE8AF68A0408F5A72474B1327A64E3E3B47D9B139230C7833B
```

## B. Aggregate tx

Apply for aggregate bonded and aggregate complete tx

### Outer transaction

```console
Property                                Types                                       Description
----------------------------------------------------------------------------------------------------------------------
transactionsHash                         Hash256 (32 bytes)                           Aggregate hash of the aggregate transaction.
payloadSize                              uint32                                         Transaction payload size in bytes
aggregateTransactionHeader_Reserved1     uint32                                         Reserved padding to align end of AggregateTransactionHeader on 8-byte boundary.
transactions                             array(Transaction, size=payloadSize)         Array of inner transactions. Other aggregate transactions are not allowed as inner transactions.
```

### Inner transaction

```console
Property                                Types                                       Description
----------------------------------------------------------------------------------------------------------------------
Size of inner tx                        uint32                                         Size all inner tx
Reserved                                uint32                                      Zeros (fixed)
Signer Publickey                        32 Bytes                                     Public key of the signer of the entity.
Inner transaction data                  sizeof(inner_tx_data)                       Normal transaction without fee and deadline. Start from property 09 of shared parts
Reserve                                 zeros                                       Add zeros to fill space after transaction
```

### Mosaic Definition Transaction schema

[Reference](https://docs.symbolplatform.com/serialization/mosaic.html#mosaic-definition-transaction)

#### Mosaic Definition Transaction (Inner tx 1)

```console
Property                                Types                                       Description
----------------------------------------------------------------------------------------------------------------------
id                                         MosaicId (uint64)                           Identifier of the mosaic.
duration                                 BlockDuration (uint64)                      Mosaic duration expressed in blocks. If set to 0, the mosaic is non-expiring.
nonce                                     uint32                                       Random nonce used to generate the mosaic id.
flags                                     MosaicFlag  (uint8)                            Mosaic flags.
divisibility                            uint8                                       Mosaic divisibility.
```

#### Mosaic Supply Change Transaction (Inner tx 2)

> *Note*: Available for normal transaction ([ref](#mosaic-supply-change-transaction-schema-0x424e))

```console
Property                                Types                                       Description
----------------------------------------------------------------------------------------------------------------------
mosaicId                                 UnresolvedMosaicId (uint64)                 Affected mosaic identifier.
delta                                     Amount (uint64)                             Amount of supply to increase or decrease.
action                                     MosaicSupplyChangeAction (uint8)            Supply change action.
```

Example:

Full raw transaction (ledger receive):

```console
E0040080FF058000002C800010F78000000080000000800000001DFB2FAA9E7F054168B0C5FCB84F4DEB62CC2B4D317D861F3168D161F54EA78B0198414180841E0000000000F9BD913906000000E5F37FE3F83F4F0A2F21E7CF25F75CF29A20D7929CBEB7EB552EDA846969281F9000000000000000460000000000000017140D44583C4BAD44C0A9DB963E315E1C425A7495271738B8F81938DDE75C400000000001984D4171243F1123B82C530A00000000000000EADF0D4407000000410000000000000017140D44583C4BAD44C0A9DB963E315E1C425A7495271738B8F81938DDE75C400000000001984D4271243F1123B82C5340420F0000000000010000000000
```

#### Parsed above tx

```console
##### Shared parts

01 -> 06    E0040080FF05
07          08000002C800010F7800000008000000080000000
08          01DFB2FAA9E7F054168B0C5FCB84F4DEB62CC2B4D317D861F3168D161F54EA78B
09 -> 10    0198
11          4141
12          80841E0000000000
13          F9BD913906000000

##### Properties parts

TX Hash               E5F37FE3F83F4F0A2F21E7CF25F75CF29A20D7929CBEB7EB552EDA846969281F
Size (all inner tx)   90000000
Reserve (outer tx)    00000000

Size(inner tx 1)      46000000
Reserve               00000000
Signer Public key     17140D44583C4BAD44C0A9DB963E315E1C425A7495271738B8F81938DDE75C40
EntityReserve1        00000000
09 ->10 (inner tx1)   0198
11(inner tx1)         4D41
Mosaic ID             71243F1123B82C53
Duration              0A00000000000000
Nonce                 EADF0D44
Flag                  07
Divisibility          00
Filling zeros         0000   (Fixed - Reserved)

Size(inner tx 2)      41000000
Reserve               00000000
Signer Public key     17140D44583C4BAD44C0A9DB963E315E1C425A7495271738B8F81938DDE75C40
EntityReserve1        00000000
09 - 10 (inner tx 2)  0198
11 (inner tx 2)       4D42
Mosaic ID             71243F1123B82C53
Delta                 40420F0000000000
Action                01
Filling zeroes        0000      (Fixed - Reserved)
More trailing zeros   00000000
```

### Multisig Account Modification schema

#### Multisig Account Modification

[Reference](https://docs.symbolplatform.com/serialization/multisig.html#multisigaccountmodificationtransaction)

```console
Property                                Types                                       Description
----------------------------------------------------------------------------------------------------------------------
minRemovalDelta                         int8                                         Number of signatures needed to remove a cosignatory.
minApprovalDelta                         int8                                         Number of signatures needed to approve a transaction.
addressAdditionsCount                   uint8                                       Number of cosignatory address additions.
addressDeletionsCount                     uint8                                       Number of cosignatory address deletions.
Reserved1                                 uint32                                         Reserved padding to align addressAdditions on 8-byte boundary.
addressAdditions                        array(UnresolvedAddress, addressAdditionsCount)         Cosignatory address additions.
addressDeletions                         array(UnresolvedAddress, addressDeletionsCount)         Cosignatory address deletions.
```

Example:

Full raw transaction (ledger receive):

```console
E0040080D9058000002C800010F78000000080000000800000001DFB2FAA9E7F054168B0C5FCB84F4DEB62CC2B4D317D861F3168D161F54EA78B0198414280841E000000000077769F5906000000043D6F6E851CAE4ED2B975AEEF61DFDF00B85BBB2503AC23DD7586E3C0B079566800000000000000680000000000000017140D44583C4BAD44C0A9DB963E315E1C425A7495271738B8F81938DDE75C40000000000198554101010200000000009817259A942F6AE0EA32B01E368687405536E61125ECF701984B730EA3B726CC12A9FAF78B4D37354FF8722DBB950137
```

#### Parsed above tx

```console
##### Shared parts

01 -> 06    E0040080D905
07          08000002C800010F78000000080000000800000000
08          1DFB2FAA9E7F054168B0C5FCB84F4DEB62CC2B4D317D861F3168D161F54EA78B
09 -> 10    0198
11          4141
12          80841E0000000000
13          77769F5906000000

##### Properties parts

TX Hash               043D6F6E851CAE4ED2B975AEEF61DFDF00B85BBB2503AC23DD7586E3C0B07956
Size (all inner tx)   68000000
Reserve (outer tx)    00000000

Size(inner tx 1)      68000000
Reserve               00000000
Signer Public key     17140D44583C4BAD44C0A9DB963E315E1C425A7495271738B8F81938DDE75C40
EntityReserve1        00000000
09 ->10 (inner tx1)   0198
11(inner tx1)         5541
minRemovalDelta       01
minApprovalDelta      01
addressAdditionsCount 02
addressDeletionsCount 00
reserve               00000000
addressAdditions1     9817259A942F6AE0EA32B01E368687405536E61125ECF701
addressAdditions2     984B730EA3B726CC12A9FAF78B4D37354FF8722DBB950137
```

### Account Metadata Transaction schema

#### Account Metadata Transaction

[Reference](https://docs.symbolplatform.com/serialization/metadata.html#accountmetadatatransaction)

```console
Property                                Types                                       Description
----------------------------------------------------------------------------------------------------------------------
targetAddress                             UnresolvedAddress (24 bytes)                Metadata target address.
scopedMetadataKey                          uint64                                        Metadata key scoped to source, target and type.
valueSizeDelta                              int16                                          Change in value size in bytes.
valueSize                                    uint16                                          Value size in bytes.
value                                      array(byte, valueSize)                        Difference between the previous value and new value. You can calculate value as xor(previous-value, new-value).


(Optional): One package just has field duration or parentId at a time. Depends on registrationType
```

Example:

Full raw transaction (ledger receive):

```console
E0040080F1058000002C800010F78000000080000000800000006C1B92391CCB41C96478471C2634C111D9E989DECD66130C0430B5B8D20117CD019841414084040000000000CFEA4287070000005F221AD2C6D297E683692CE332B24157057E6FB43A832F18C13495EC49544E0880000000000000007F0000000000000017140D44583C4BAD44C0A9DB963E315E1C425A7495271738B8F81938DDE75C40000000000198444198F2A5E8E063AD1A9085EF5B5167E2F1A5645C48FA2C02497AEAFC0DA38583AB2B002B0074686973206973207468652076616C7565206669656C64206F66206163636F756E74206D6574616461746100
```

#### Parsed above tx

```console
##### Shared parts
01 -> 06    E0040080F105
07          8000002C800010F7800000008000000080000000
08          6C1B92391CCB41C96478471C2634C111D9E989DECD66130C0430B5B8D20117CD
09 -> 10    0198
11          4141
12          4084040000000000
13          CFEA428707000000

##### Properties parts
TX Hash               5F221AD2C6D297E683692CE332B24157057E6FB43A832F18C13495EC49544E08
Size (all inner tx)   80000000
Reserve (outer tx)    00000000

Size (inner tx 1)     7F000000
Reserve               00000000
Signer Public Key     17140D44583C4BAD44C0A9DB963E315E1C425A7495271738B8F81938DDE75C40
EntityReserve1        00000000
09 ->10 (inner tx1)   0198
11(inner tx1)         4441
targetAddress         98F2A5E8E063AD1A9085EF5B5167E2F1A5645C48FA2C0249
scopedMetadataKey     7AEAFC0DA38583AB
valueSizeDelta        2B00
valueSize             2B00
value                 74686973206973207468652076616C7565206669656C64206F66206163636F756E74206D65746164617461
Filling zeroes        00
```
