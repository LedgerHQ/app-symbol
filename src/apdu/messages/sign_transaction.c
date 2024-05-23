/*******************************************************************************
*    XYM Wallet
*    (c) 2020 Ledger
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
#include "sign_transaction.h"
#include <os.h>
#include "global.h"
#include "xym/xym_helpers.h"
#include "ui/main/idle_menu.h"
#include "ui/transaction/review_menu.h"
#include "transaction/transaction.h"
#include "printers.h"
#include "io.h"
#include "crypto.h"

#define PREFIX_LENGTH   4

buffer_t        rawTxData;  ///< transaction data is extracted from this buffer 
fields_array_t  fields;     ///< extracted data from rawTxData is used to fill this structure, which is displayed to user for confirmation

ApduResponse_t handle_packet_content( const buffer_t* buffer, const bool lastPacket );


void sign_transaction()
{
    if( signState != PENDING_REVIEW ) 
    {
        reset_transaction_context();
        display_idle_menu();
        return;
    }

    // Abort if we accidentally end up here again after the transaction has already been signed
    if( rawTxData.ptr == NULL ) 
    {
        display_idle_menu();
        return;
    }

    cx_ecfp_private_key_t privateKey;
    unsigned char signature[IO_APDU_BUFFER_SIZE];
    uint32_t sigLength = 0;

    io_seproxyhal_io_heartbeat();

    BEGIN_TRY
    {
        TRY 
        {
            // get private key from bip32 path
            crypto_derive_private_key( transactionContext.bip32Path, transactionContext.pathLength, transactionContext.curve, &privateKey );
            io_seproxyhal_io_heartbeat();

            // sign transaction
            sigLength = (uint32_t) cx_eddsa_sign( &privateKey, CX_LAST, CX_SHA512, transactionContext.rawTx,
                                                   transactionContext.rawTxLength, NULL, 0, signature,
                                                   IO_APDU_BUFFER_SIZE, NULL );
        }
        CATCH_OTHER(e) 
        {
            THROW(e);
        }
        FINALLY 
        {
            explicit_bzero( &privateKey, sizeof(privateKey) );

            // Always reset transaction context after a transaction has been signed
            reset_transaction_context();
        }
    }
    END_TRY;

    // send response
    buffer_t response = { signature, sigLength, 0 };
    io_send_response( &response, OK );
    explicit_bzero( signature,   sizeof(signature)  );

    display_review_done(true);
}

void reject_transaction() 
{
    if (signState != PENDING_REVIEW) 
    {
        reset_transaction_context();
        display_idle_menu();
        return;
    }

    // notify of rejected transaction
    handle_error( TRANSACTION_REJECTED );

    display_review_done(false);
}

bool isFirst(uint8_t p1) 
{
	return (p1 & P1_MASK_ORDER) == 0;
}

bool hasMore(uint8_t p1) 
{
	return (p1 & P1_MASK_MORE) != 0;
}

ApduResponse_t handle_first_packet( const ApduCommand_t* cmd ) 
{
    // check that its the first packet
    if( !isFirst(cmd->p1) )
    {
        return INVALID_SIGNING_PACKET_ORDER;
    }

    // Reset old transaction data that might still remain
    reset_transaction_context();

    // check that p2 is set to either SECP256K1 or ED25519
    if( ( ((cmd->p2 & P2_SECP256K1) == 0) && ((cmd->p2 & P2_ED25519) == 0) ) ||
        ( ((cmd->p2 & P2_SECP256K1) != 0) && ((cmd->p2 & P2_ED25519) != 0) )    )
    {
        return INVALID_P1_OR_P2;
    }

    // convert apdu data to bip32 path
    const buffer_t buffer = { cmd->data, cmd->lc, 0 };
    transactionContext.pathLength = buffer_get_bip32_path( &buffer, transactionContext.bip32Path );
    if( 0 == transactionContext.pathLength )
    {
        return INVALID_BIP32_PATH_LENGTH;
    }
    
    // set curve
    transactionContext.curve = (((cmd->p2 & P2_ED25519) != 0) ? CURVE_Ed25519 : CURVE_256K1);


    const size_t bip32PathSize = transactionContext.pathLength*4+1;
    buffer_t serializedData = { &cmd->data[bip32PathSize], cmd->lc-bip32PathSize, 0 }; // buffer without the bip32 path
    return handle_packet_content( &serializedData, !hasMore(cmd->p1) );
}

ApduResponse_t handle_subsequent_packet( const ApduCommand_t* cmd ) 
{
    if (isFirst(cmd->p1)) 
    {
        THROW( INVALID_SIGNING_PACKET_ORDER );
    }
    buffer_t serializedData = { cmd->data, cmd->lc, 0 }; // buffer without the bip32 path
    return handle_packet_content( &serializedData, !hasMore(cmd->p1) );
}

ApduResponse_t handle_packet_content( const buffer_t* buffer, const bool lastPacket ) 
{
    uint16_t totalLength = PREFIX_LENGTH + transactionContext.rawTxLength + buffer->size;
    if( totalLength > MAX_RAW_TX )
    {
        // Abort if the user is trying to sign a too large transaction
        return SIGNING_DATA_TOO_LARGE;
    }

    // Append received data to stored transaction data
    memcpy( transactionContext.rawTx + transactionContext.rawTxLength, buffer->ptr, buffer->size );
    transactionContext.rawTxLength += buffer->size;

    if( !lastPacket )
    {
        // Reply to sender with status OK, so that next packet is sent
        signState = WAITING_FOR_MORE;
        const int succ = io_send_response(NULL, OK);
        return ( (succ != -1) ? OK : INTERNAL_ERROR );
    }
    else
    {
        // All data received, prepare transaction fields to be presented to user
        signState = PENDING_REVIEW;

        rawTxData.ptr    = transactionContext.rawTx;
        rawTxData.size   = transactionContext.rawTxLength;
        rawTxData.offset = 0;

        int status = parse_txn_context(&rawTxData, &fields);

        switch( status )
        {
            case E_TOO_MANY_FIELDS:
            {
                // Abort if there are too many fields to show on Ledger device
                return TOO_MANY_TRANSACTION_FIELDS;
            }
            case E_NOT_ENOUGH_DATA:
            case E_INVALID_DATA:
            {
                return INVALID_SIGNING_DATA;
            }
            default: // E_SUCCESS
                break;
        }

        review_transaction(&fields, sign_transaction, reject_transaction);

        return OK;
    }
}

int handle_sign( const ApduCommand_t* cmd ) 
{
    ApduResponse_t result;

    switch( signState )
    {
        case IDLE:
        {
            result = handle_first_packet( cmd );
            break;
        }
        case WAITING_FOR_MORE:
        {
            result = handle_subsequent_packet( cmd );
            break;
        }
        default:
        {
            THROW(INVALID_INTERNAL_SIGNING_STATE);
        }
    }


    if( OK != result )
    {
        return handle_error( result );
    }

    return 0;
}
