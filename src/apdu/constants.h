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
#ifndef LEDGER_APP_XYM_CONSTANTS_H
#define LEDGER_APP_XYM_CONSTANTS_H

#define CLA             0xE0
#define P1_CONFIRM      0x01
#define P1_NON_CONFIRM  0x00
#define P2_NO_CHAINCODE 0x00
#define P2_CHAINCODE    0x01
#define P1_MASK_ORDER   0x01u
#define P1_MASK_MORE    0x80u
#define P2_SECP256K1    0x40u
#define P2_ED25519      0x80u

#define OFFSET_CLA   0  // Offset of instruction class
#define OFFSET_INS   1  // Offset of instruction code
#define OFFSET_P1    2  // Offset of instruction parameter 1
#define OFFSET_P2    3  // Offset of instruction parameter 2
#define OFFSET_LC    4  // Offset of command data length
#define OFFSET_CDATA 5  // Offset of command data

#endif  // LEDGER_APP_XYM_CONSTANTS_H
