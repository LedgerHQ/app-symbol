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
#include "get_public_key.h"
#include "apdu/global.h"
#include "xym/xym_helpers.h"
#include "ui/main/idle_menu.h"
#include "ui/address/address_ui.h"
#include "types.h"
#include "io.h"
#include "crypto.h"

uint8_t G_xym_public_key[ XYM_PUBLIC_KEY_LENGTH ];

typedef struct 
{
    bool        confirmTransaction;
    uint8_t     bip32PathLength;
    uint32_t    bip32Path[ MAX_BIP32_PATH ];
    uint8_t     networkType;
    CurveType_t curveType;
} KeyData_t;


/**
 * Sends public key to host in an APDU packet
 */
int send_public_key()
{
    size_t tx = 0;
    G_io_apdu_buffer[tx++] = XYM_PUBLIC_KEY_LENGTH;
    memcpy(G_io_apdu_buffer + tx, G_xym_public_key, XYM_PUBLIC_KEY_LENGTH);
    tx += XYM_PUBLIC_KEY_LENGTH;
    buffer_t buffer = { G_io_apdu_buffer, tx, 0 };

    return io_send_response( &buffer, OK );
}

/**
 * Ledger Bolos callback for when user confirms address
 */
void on_address_confirmed() 
{
    send_public_key();

    display_address_confirmation_done(true);
}


/**
 * Ledger Bolos callback for when user rejects address
 */
void on_address_rejected() 
{
    io_send_error(ADDRESS_REJECTED);

    display_address_confirmation_done(false);
}


/**
 * Extracts key data used for calculating public key, from APDU parameters, and returns it in 'keyData'.
 * 
 */
ApduResponse_t extract_parameters( const uint8_t p1, const uint8_t p2, uint8_t* data, const uint8_t dataLength, KeyData_t* keyData )
{
    // check length of data is correct
    if( dataLength != XYM_PKG_GETPUBLICKEY_LENGTH )
    {
        return INVALID_PKG_KEY_LENGTH;
    }

    // check that p1 is set to either to confirm or not to confirm transaction by user
    if( (p1 != P1_CONFIRM) && (p1 != P1_NON_CONFIRM) )
    {
        return INVALID_P1_OR_P2;
    }

    // check that p2 is set to either SECP256K1 or ED25519
    if( ( ((p2 & P2_SECP256K1) == 0) && ((p2 & P2_ED25519) == 0) ) ||
        ( ((p2 & P2_SECP256K1) != 0) && ((p2 & P2_ED25519) != 0) )    )
    {
        return INVALID_P1_OR_P2;
    }

    // convert apdu data to bip32 path
    const buffer_t buffer = { data, dataLength, 0 };
    uint8_t bip32PathLength = buffer_get_bip32_path( &buffer, keyData->bip32Path );
    if( 0 == bip32PathLength )
    {
        return INVALID_BIP32_PATH_LENGTH;
    }

    // prepare output
    keyData->confirmTransaction = (p1 == P1_CONFIRM);
    keyData->bip32PathLength    = bip32PathLength;
    keyData->networkType        = data[bip32PathLength*4+1];                                   
    keyData->curveType          = (((p2 & P2_ED25519) != 0) ? CURVE_Ed25519 : CURVE_256K1);

    return OK;
}


/**
 * Calculates and returns a public key which corresponds to bip32 path in 'keyData'
 * 
 */
void get_public_key( KeyData_t* keyData, uint8_t key[ XYM_PUBLIC_KEY_LENGTH ], char address[ XYM_PRETTY_ADDRESS_LENGTH+1 ] )
{
    cx_ecfp_private_key_t privateKey;

    // ensure a I/O channel is not timing out
    io_seproxyhal_io_heartbeat();

    BEGIN_TRY 
    {
        TRY 
        {            
            // get private key
            crypto_derive_private_key( keyData->bip32Path, keyData->bip32PathLength, keyData->curveType, &privateKey );

            // ensure a I/O channel is not timing out
            io_seproxyhal_io_heartbeat(); 

            // generate public key from private key
            cx_ecfp_public_key_t  publicKey;
            cx_ecfp_generate_pair2( CX_CURVE_Ed25519, &publicKey, &privateKey, 1, CX_SHA512 );
            explicit_bzero( &privateKey, sizeof(privateKey) );
            
            // ensure a I/O channel is not timing out
            io_seproxyhal_io_heartbeat(); 

            // convert key to xym format
            xym_public_key_and_address( &publicKey,
                                         keyData->networkType,
                                         key,
                                         address,
                                         XYM_PRETTY_ADDRESS_LENGTH + 1 );

            // ensure a I/O channel is not timing out
            io_seproxyhal_io_heartbeat(); 

            address[XYM_PRETTY_ADDRESS_LENGTH] = '\0';
        }
        CATCH_OTHER(e) 
        {
            THROW(e);
        }
        FINALLY 
        {
            explicit_bzero( &privateKey, sizeof(privateKey) );
        }
    }
    END_TRY;
}


int handle_public_key( const ApduCommand_t* cmd )
{
    // extract key data used for calculating public key, from APDU parameters
    KeyData_t keyData;
    const ApduResponse_t result = extract_parameters( cmd->p1, cmd->p2, cmd->data, cmd->lc, &keyData );
    if( OK != result )
    {
        return handle_error(result);
    }

    // get the public key
    char address[ XYM_PRETTY_ADDRESS_LENGTH+1 ];
    get_public_key( &keyData, G_xym_public_key, address );

    // send public key or ask for user confirmation
    if( !keyData.confirmTransaction ) 
    {
        return send_public_key();
    }
    else 
    {
        display_address_confirmation_ui( address, on_address_confirmed, on_address_rejected );
        return 0; ///< this will make the 'io_receive()' call in the main loop block until user either confirms or rejects address.
    }
}
