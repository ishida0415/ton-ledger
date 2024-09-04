/*****************************************************************************
 *   Ledger App Boilerplate.
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

#include <stdint.h>   // uint*_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool
#include <string.h>   // memmove

#include "os.h"
#include "cx.h"

#include "address.h"

#include "transaction/types.h"

const uint8_t root_header[] = {
    // Cell data and refs descriptor
    0x02, 0x01, 0x34, 0x00, 0x07, 0x00, 0x00,
    // Code cell hash
    0xfe, 0xb5, 0xff, 0x68, 0x20, 0xe2, 0xff, 0x0d, 
    0x94, 0x83, 0xe7, 0xe0, 0xd6, 0x2c, 0x81, 0x7d,
    0x84, 0x67, 0x89, 0xfb, 0x4a, 0xe5, 0x80, 0xc8,
    0x78, 0x86, 0x6d, 0x95, 0x9d, 0xab, 0xd5, 0xc0
};

const uint8_t data_header[] = {
    0x00, 0x51, // Cell header
    0x00, 0x00, 0x00, 0x00, // Seqno
    0x29, 0xa9, 0xa3, 0x17, // Wallet ID
};

const uint8_t data_tail[] = {
    0x40 // zero bit + padding
};

bool address_from_pubkey(const uint8_t public_key[static 32], uint8_t *out, size_t out_len) {
    uint8_t address[32] = {0};
    uint8_t inner[32] = {0};
    cx_sha256_t state;

    if (out_len < ADDRESS_LEN) {
        return false;
    }

    // Hash init data cell bits
    cx_sha256_init(&state);
    cx_hash((cx_hash_t *) &state, 0, data_header, sizeof(data_header), NULL, 0);
    cx_hash((cx_hash_t *) &state, 0, public_key, 32, NULL, 0);
    cx_hash((cx_hash_t *) &state, CX_LAST, data_tail, sizeof(data_tail), inner, sizeof(inner));

    // Hash root
    cx_sha256_init(&state);
    cx_hash((cx_hash_t *) &state, 0, root_header, sizeof(root_header), NULL, 0);
    cx_hash((cx_hash_t *) &state, CX_LAST, inner, sizeof(inner), address, sizeof(address));

    // Copy results
    memmove(out, address, ADDRESS_LEN);

    return true;
}
