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
#ifndef LEDGER_APP_XYM_GETPUBLICKEY_H
#define LEDGER_APP_XYM_GETPUBLICKEY_H

#include "types.h"

/**
 * Processes the APDU command, extracts the key data and displays
 * the corresponding public key to user for verification
 *
 * @param[in] cmd
 *   Structured APDU command (CLA, INS, P1, P2, Lc, Command data).
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handle_public_key(const ApduCommand_t* cmd);

#endif  // LEDGER_APP_XYM_GETPUBLICKEY_H
