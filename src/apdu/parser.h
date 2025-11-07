#pragma once

#include <stddef.h>   // size_t
#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool

#include "types.h"

/**
 * Parse APDU command from byte buffer.
 *
 * @param[in]  buf
 *   Byte buffer with raw APDU command.
 * @param[in]  buf_len
 *   Length of byte buffer.
 *
 *
 * @param[out] cmd
 *   Structured APDU command (CLA, INS, P1, P2, Lc, Command data).
 *
 * @return true if success, false otherwise.
 */
bool apdu_parser(uint8_t* buf, size_t buf_len, ApduCommand_t* cmd);
