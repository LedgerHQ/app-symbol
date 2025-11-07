#include <stdbool.h>
#include "io.h"
#include "os.h"
#include "types.h"  //TODO: use constants.h instead
#include "os_io_seproxyhal.h"

/**
 * Enumeration for the status of IO.
 */
enum io_state_e {
    UNINITIALIZED,  ///< 'io_init()' not called yet
    READY,          ///< ready for new event
    RECEIVED,       ///< data received
    WAITING         ///< waiting
};

uint32_t G_output_len;
enum io_state_e G_io_state = UNINITIALIZED;

/**
 * Copy bytes from buffer without moving offset.
 *
 * @param[in]  buffer
 *   Pointer to input buffer struct.
 * @param[out] out
 *   Pointer to output byte buffer.
 * @param[in]  out_len
 *   Length of output byte buffer.
 *
 * @return true if success, false otherwise.
 *
 */
bool buffer_copy(const buffer_t *buffer, uint8_t *out, size_t out_len) {
    if (buffer->size - buffer->offset > out_len) {
        return false;
    }

    memmove(out, buffer->ptr + buffer->offset, buffer->size - buffer->offset);

    return true;
}

/**
 * Write 16-bit unsigned integer value as Big Endian.
 *
 * @param[out] ptr
 *   Pointer to output byte buffer.
 * @param[in]  offset
 *   Offset in the output byte buffer.
 * @param[in]  value
 *   16-bit unsigned integer to write in output byte buffer as Big Endian.
 *
 */
void write_u16_be(uint8_t *ptr, size_t offset, uint16_t value) {
    ptr[offset + 0] = (uint8_t) (value >> 8);
    ptr[offset + 1] = (uint8_t) (value >> 0);
}

void io_init() {
    G_output_len = 0;
    G_io_state = READY;
}

int io_receive_command() {
    int ret;

    switch (G_io_state) {
        case READY: {
            G_io_state = RECEIVED;
            ret = io_exchange(CHANNEL_APDU, G_output_len);
            break;
        }
        case RECEIVED: {
            G_io_state = WAITING;
            ret = io_exchange(CHANNEL_APDU | IO_ASYNCH_REPLY, G_output_len);
            G_io_state = RECEIVED;
            break;
        }
        case WAITING: {
            G_io_state = READY;
            ret = -1;
            break;
        }
        case UNINITIALIZED: {
            ret = -1;
            break;
        }
    }

    return ret;
}

int io_send_response(const buffer_t *rdata, uint16_t sw) {
    if (rdata != NULL) {
        if (rdata->size - rdata->offset > IO_APDU_BUFFER_SIZE - 2 ||
            !buffer_copy(rdata, G_io_apdu_buffer, sizeof(G_io_apdu_buffer))) {
            return io_send_error(WRONG_RESPONSE_LENGTH);
        }

        G_output_len = rdata->size - rdata->offset;
        PRINTF("<= SW=%04X | RData=%.*H\n", sw, rdata->size, rdata->ptr);
    } else {
        G_output_len = 0;
        PRINTF("<= SW=%04X | RData=\n", sw);
    }

    write_u16_be(G_io_apdu_buffer, G_output_len, sw);
    G_output_len += 2;

    int ret;
    switch (G_io_state) {
        case READY: {
            ret = -1;
            break;
        }
        case RECEIVED: {
            G_io_state = READY;
            ret = 0;
            break;
        }
        case WAITING: {
            ret = io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, G_output_len);
            G_output_len = 0;
            G_io_state = READY;
            break;
        }
        case UNINITIALIZED: {
            ret = -1;
            break;
        }
    }

    return ret;
}

int io_send_error(uint16_t sw) {
    return io_send_response(NULL, sw);
}

unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
    switch (channel & ~(IO_FLAGS)) {
        case CHANNEL_KEYBOARD: {
            break;
        }
        // multiplexed io exchange over a SPI channel and TLV encapsulated protocol
        case CHANNEL_SPI: {
            if (tx_len) {
                io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

                if (channel & IO_RESET_AFTER_REPLIED) {
                    reset();
                }
                return 0;  // nothing received from the master so far (it's a tx
                           // transaction)
            } else {
                return io_seproxyhal_spi_recv(G_io_apdu_buffer, sizeof(G_io_apdu_buffer), 0);
            }
        }
        default: {
            THROW(INVALID_PARAMETER);
        }
    }

    return 0;
}
