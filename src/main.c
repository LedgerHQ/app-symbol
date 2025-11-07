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

#include "os.h"
#include <os_io_seproxyhal.h>
#include <ux.h>
#include "bagl_utils.h"
#include "apdu/entry.h"
#include "apdu/global.h"
#include "ui/main/idle_menu.h"
#include "ui/address/address_ui.h"
#include "types.h"
#include "io.h"
#include "parser.h"

// IO_SEPROXYHAL_BUFFER_SIZE_B define in Makefile
unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

ux_state_t G_ux;
bolos_ux_params_t G_ux_params;


void xym_main(void) 
{
    io_init();

    while( true )
    {
        BEGIN_TRY 
        {
            TRY 
            {
                // Receive command bytes in G_io_apdu_buffer
                const int size = io_receive_command();
                if( size < 0 )
                {
                    handle_error( NO_APDU_RECEIVED );
                    return;
                }

                // Parse APDU command from G_io_apdu_buffer
                ApduCommand_t cmd;
                memset( &cmd, 0, sizeof(cmd) );

                if( !apdu_parser(G_io_apdu_buffer, size, &cmd) ) 
                {
                    PRINTF("=> /!\\ BAD LENGTH: %.*H\n", size, G_io_apdu_buffer);
                    handle_error( WRONG_APDU_DATA_LENGTH );
                    continue;
                }

                //PRINTF( "New APDU: CLA=%02X | INS=%02X | P1=%02X | P2=%02X | Lc=%02X | CData=%.*H\n", cmd.cla, cmd.ins, cmd.p1, cmd.p2, cmd.lc, cmd.data );
                PRINTF("New APDU: %.*h \n", cmd.lc, cmd.data);


                handle_apdu( &cmd );
            }
            CATCH(EXCEPTION_IO_RESET)
            {
                THROW(EXCEPTION_IO_RESET);
            }
            CATCH_OTHER(e) 
            {
                handle_error( e );
            }
            FINALLY 
            {
            }
        }
        END_TRY;
    }
}

#ifdef HAVE_BAGL
// override point, but nothing more to do
void io_seproxyhal_display(const bagl_element_t *element) 
{
    io_seproxyhal_display_default(element);
}
#endif // HAVE_BAGL

unsigned char io_event(unsigned char channel) 
{
    UNUSED(channel);

    // nothing done with the event, throw an error on the transport layer if
    // needed

    // can't have more than one tag in the reply, not supported yet.
    switch (G_io_seproxyhal_spi_buffer[0]) {
    case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT:
#ifdef HAVE_BAGL
        UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
#endif  // HAVE_BAGL
        break;

    case SEPROXYHAL_TAG_STATUS_EVENT:
        if (G_io_apdu_media == IO_APDU_MEDIA_USB_HID &&
            !(U4BE(G_io_seproxyhal_spi_buffer, 3) &
              SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED)) {
            THROW(EXCEPTION_IO_RESET);
        }
        /* fallthrough */

    case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
#ifdef HAVE_BAGL
        UX_DISPLAYED_EVENT({});
#endif  // HAVE_BAGL
#ifdef HAVE_NBGL
        UX_DEFAULT_EVENT();
#endif  // HAVE_NBGL
        break;

#ifdef HAVE_NBGL
    case SEPROXYHAL_TAG_FINGER_EVENT:
        UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
        break;
#endif  // HAVE_NBGL

    case SEPROXYHAL_TAG_TICKER_EVENT:
        UX_TICKER_EVENT(G_io_seproxyhal_spi_buffer, {
            if (UX_ALLOWED) {
                // redisplay screen
                UX_REDISPLAY();
            }
        });
        break;

    default:
        UX_DEFAULT_EVENT();
        break;
    }

    // close the event if not done previously (by a display or whatever)
    if (!io_seproxyhal_spi_is_status_sent()) {
        io_seproxyhal_general_status();
    }

    // command has been processed, DO NOT reset the current APDU transport
    return 1;
}

void app_exit(void) {
    BEGIN_TRY_L(exit) {
        TRY_L(exit) {
            os_sched_exit(1);
        }
        FINALLY_L(exit) {
        }
    }
    END_TRY_L(exit);
}

__attribute__((section(".boot"))) int main(void) {
    // exit critical section
    __asm volatile("cpsie i");

    // ensure exception will work as planned
    os_boot();

    for (;;) {
        reset_transaction_context();

        UX_INIT()
        BEGIN_TRY {
            TRY {
                io_seproxyhal_init();

#ifdef HAVE_BLE
                // grab the current plane mode setting
                G_io_app.plane_mode = os_setting_get(OS_SETTING_PLANEMODE, NULL, 0);
#endif // HAVE_BLE

                USB_power(0);
                USB_power(1);

                display_idle_menu();

#ifdef HAVE_BLE
                BLE_power(0, NULL);
                BLE_power(1, NULL);
#endif // HAVE_BLE

                xym_main();
            }
            CATCH(EXCEPTION_IO_RESET) {
                CLOSE_TRY;
                // reset IO and UX
                continue;
            }
            CATCH_ALL {
                CLOSE_TRY;
                break;
            }
            FINALLY {
            }
        }
        END_TRY;
    }
    app_exit();

    return 0;
}
