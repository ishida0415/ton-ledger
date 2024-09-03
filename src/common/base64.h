#pragma once

#include <stddef.h>   // size_t
#include <stdint.h>   // uint*_t

/**
 * Encode input bytes in base 64.
 *
 * @see https://tools.ietf.org/html/draft-msporny-base58-02
 *
 * @param[in]  in
 *   Pointer to input byte buffer.
 * @param[in]  in_len
 *   Length of the input byte buffer.
 * @param[out] out
 *   Pointer to output string buffer.
 * @param[in]  out_len
 *   Maximum length to write in output byte buffer.
 *
 * @return number of bytes encoded, -1 otherwise.
 *
 */
size_t base64_encode(const uint8_t *data, size_t data_length, char *out, size_t out_len);