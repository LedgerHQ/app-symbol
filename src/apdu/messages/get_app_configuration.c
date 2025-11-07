/*******************************************************************************
 *    XYM Wallet
 *    (c) 2017 Ledger
 *    (c) 2020 FDS
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
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
#include "get_app_configuration.h"
#include "os.h"
#include "io.h"
#include "types.h"

/*
 * LEDGER_MAJOR_VERSION, LEDGER_MINOR_VERSION, LEDGER_PATCH_VERSION defined in Makefile
 */
int handle_app_configuration() {
    unsigned char data[4];
    data[0] = 0x00;
    data[1] = LEDGER_MAJOR_VERSION;
    data[2] = LEDGER_MINOR_VERSION;
    data[3] = LEDGER_PATCH_VERSION;

    buffer_t buffer = {data, 4, 0};
    return io_send_response(&buffer, OK);
}
