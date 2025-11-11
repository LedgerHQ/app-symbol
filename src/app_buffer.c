#include "app_buffer.h"

bool buffer_seek(buffer_t* buffer, size_t offset) {
    if (buffer->offset + offset < buffer->offset ||  // overflow
        buffer->offset + offset > buffer->size)      // exceed buffer size
    {
        return false;
    }

    buffer->offset += offset;

    return true;
}

const uint8_t* buffer_offset_ptr(buffer_t* buffer) {
    return (buffer->ptr + buffer->offset);
}

const uint8_t* buffer_offset_ptr_and_seek(buffer_t* buffer, size_t n) {
    const uint8_t* out = buffer_offset_ptr(buffer);
    const bool succ = buffer_seek(buffer, n);

    if (!succ) {
        out = NULL;
    }

    return out;
}

uint8_t buffer_get_bip32_path(const buffer_t* buffer, uint32_t bip32Path[MAX_BIP32_PATH]) {
    // check that bip32 path length is correct
    uint8_t bip32PathLength = buffer->ptr[0];
    if ((bip32PathLength < 1) || (bip32PathLength > MAX_BIP32_PATH)) {
        return 0;
    }

    // convert data to bip32 paths
    size_t dataIdx = 1;
    for (size_t pathIdx = 0; pathIdx < bip32PathLength; pathIdx++, dataIdx += 4) {
        // change endianness
        bip32Path[pathIdx] = (buffer->ptr[dataIdx + 0] << 24) | (buffer->ptr[dataIdx + 1] << 16) |
                             (buffer->ptr[dataIdx + 2] << 8) | (buffer->ptr[dataIdx + 3] << 0);
    }

    return bip32PathLength;
}
