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
#include <string.h>   // memset, explicit_bzero
#include <stdbool.h>  // bool

#include "crypto.h"

#include "globals.h"

int crypto_derive_private_key(cx_ecfp_private_key_t *private_key,
                              const uint32_t *bip32_path,
                              uint8_t bip32_path_len) {
    uint8_t raw_private_key[32] = {0};

    BEGIN_TRY {
        TRY {
            // derive the seed with bip32_path
            os_perso_derive_node_bip32_seed_key(HDW_ED25519_SLIP10,
                                                CX_CURVE_Ed25519,
                                                bip32_path,
                                                bip32_path_len,
                                                raw_private_key,
                                                NULL,
                                                (unsigned char *) "ed25519 seed",
                                                12);

            // new private_key from raw
            cx_ecfp_init_private_key(CX_CURVE_Ed25519,
                                     raw_private_key,
                                     sizeof(raw_private_key),
                                     private_key);
        }
        CATCH_OTHER(e) {
            THROW(e);
        }
        FINALLY {
            explicit_bzero(&raw_private_key, sizeof(raw_private_key));
        }
    }
    END_TRY;

    return 0;
}

int crypto_init_public_key(cx_ecfp_private_key_t *private_key,
                           cx_ecfp_public_key_t *public_key,
                           uint8_t raw_public_key[static 32]) {
    // generate corresponding public key
    cx_ecfp_generate_pair(CX_CURVE_Ed25519, public_key, private_key, 1);

    // Convert to NaCL format
    for (int i = 0; i < 32; i++) {
        raw_public_key[i] = public_key->W[64 - i];
    }
    if ((public_key->W[32] & 1) != 0) {
        raw_public_key[31] |= 0x80;
    }

    return 0;
}

int crypto_sign_message() {
    cx_ecfp_private_key_t private_key = {0};
    int sig_len = 0;

    // derive private key according to BIP32 path
    crypto_derive_private_key(&private_key, G_context.bip32_path, G_context.bip32_path_len);

    BEGIN_TRY {
        TRY {
            sig_len = cx_eddsa_sign(&private_key,
                                    CX_LAST,
                                    CX_SHA512,
                                    G_context.tx_info.m_hash,
                                    sizeof(G_context.tx_info.m_hash),
                                    NULL,
                                    0,
                                    G_context.tx_info.signature,
                                    sizeof(G_context.tx_info.signature),
                                    NULL);
            PRINTF("Signature: %.*H\n", sig_len, G_context.tx_info.signature);
        }
        CATCH_OTHER(e) {
            THROW(e);
        }
        FINALLY {
            explicit_bzero(&private_key, sizeof(private_key));
        }
    }
    END_TRY;

    if (sig_len < 0) {
        return -1;
    }

    G_context.tx_info.signature_len = sig_len;

    return 0;
}
