/*****************************************************************************
 *   (c) 2020 Ledger SAS.
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
 *****************************************************************************/

#pragma once
#include <string.h>
#include <stdint.h>
#include "app_buffer.h"

/**
 * A set of higher level io commands that hide the complexity of
 * 'io_exchange()' and flags 'IO_RETURN_AFTER_TX' and 'IO_ASYNCH_REPLY',
 * by implementing a simple state machine shown below:
 *
 *
 *   _______________  init()  _______   receive()        __________
 *  | Uninitialized |------->| Ready |----------------->| Received |
 *  |_______________|     ┌--|_______|<------------┐----|__________|
 *   |                    |              send()    |             |
 *   |             send() |                        |             | receive()
 *   |                    |   _______             _|_______      |
 *   | send()/receive()   └->| Error |<----------| Waiting |<----┘
 *   └---------------------->|_______| receive() |_________|
 *
 *
 * The main commands are send() and receive() and calling them will
 * result in a state changes. The only blocking state is the 'Waiting'
 * state. When calling 'receive()' and in the 'Ready' state, the function
 * returns immediately with the APDU buffer. A subsequent 'receive()' call
 * without a previous 'send()' call, will block the call until another
 * thread/interrupt calls 'send()'
 */

/**
 * Must be called once in the main method, before calling any other io commands
 *
 */
void io_init();

/**
 * Receive APDU command in G_io_apdu_buffer.
 *
 * @return zero or positive integer if success, -1 otherwise.
 *
 */
int io_receive_command(void);

/**
 * Send APDU response (response data + status word) by filling
 * G_io_apdu_buffer.
 *
 * @param[in] rdata
 *   Buffer with APDU response data.
 * @param[in] sw
 *   Status word of APDU response.
 *
 * @return zero or positive integer if success, -1 otherwise.
 *
 */
int io_send_response(const buffer_t *rdata, uint16_t sw);

/**
 * Send APDU response (only status word) by filling
 * G_io_apdu_buffer.
 *
 * @param[in] sw
 *   Status word of APDU response.
 *
 * @return zero or positive integer if success, -1 otherwise.
 *
 */
int io_send_error(uint16_t sw);
