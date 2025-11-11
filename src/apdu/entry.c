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
#include "entry.h"
#include "global.h"
#include "get_public_key.h"
#include "sign_transaction.h"
#include "get_app_configuration.h"

static unsigned char lastINS = 0;

int handle_apdu(const command_t* cmd) {
    if (cmd->cla != CLA) {
        return handle_error(SWO_INVALID_CLA);
    }

    // Reset transaction context before starting to parse a new APDU message type.
    // This helps protect against "Instruction Change" attacks
    if (cmd->ins != lastINS) {
        reset_transaction_context();
    }

    lastINS = cmd->ins;

    switch (cmd->ins) {
        case GET_PUBLIC_KEY:
            return handle_public_key(cmd);

        case SIGN_TX:
            return handle_sign(cmd);

        case GET_VERSION:
            return handle_app_configuration();

        default:
            return handle_error(SWO_INVALID_INS);
    }
}
