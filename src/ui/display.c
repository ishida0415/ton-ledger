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

#pragma GCC diagnostic ignored "-Wformat-invalid-specifier"  // snprintf
#pragma GCC diagnostic ignored "-Wformat-extra-args"         // snprintf

#include <stdbool.h>  // bool
#include <string.h>   // memset

#include "os.h"
#include "ux.h"
#include "glyphs.h"

#include "display.h"
#include "constants.h"
#include "../globals.h"
#include "../io.h"
#include "../sw.h"
#include "../address.h"
#include "action/validate.h"
#include "../transaction/types.h"
#include "../common/bip32.h"
#include "../common/format.h"
#include "../common/base64.h"

static action_validate_cb g_validate_callback;
static char g_operation[64];
static char g_amount[30];
static char g_address[49];
static char g_payload[512];

// Step with icon and text
UX_STEP_NOCB(ux_display_confirm_addr_step, pn, {&C_icon_eye, "Confirm Address"});
// Step with title/text for address
UX_STEP_NOCB(ux_display_address_step,
             bnnn_paging,
             {
                 .title = "Address",
                 .text = g_address,
             });
// Step with approve button
UX_STEP_CB(ux_display_approve_step,
           pb,
           (*g_validate_callback)(true),
           {
               &C_icon_validate_14,
               "Approve",
           });
// Step with reject button
UX_STEP_CB(ux_display_reject_step,
           pb,
           (*g_validate_callback)(false),
           {
               &C_icon_crossmark,
               "Reject",
           });

// FLOW to display address and BIP32 path:
// #1 screen: eye icon + "Confirm Address"
// #3 screen: display address
// #4 screen: approve button
// #5 screen: reject button
UX_FLOW(ux_display_pubkey_flow,
        &ux_display_confirm_addr_step,
        &ux_display_address_step,
        &ux_display_approve_step,
        &ux_display_reject_step);

int ui_display_address(uint8_t flags) {
    // Check state
    if (G_context.req_type != CONFIRM_ADDRESS || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    // Format address
    memset(g_address, 0, sizeof(g_address));
    uint8_t address[ADDRESS_LEN] = {0};
    bool bounceable = true;
    bool testnet = false;
    uint8_t chain = 0;
    if (flags & 0x01) {
        bounceable = false;
    }
    if (flags & 0x02) {
        testnet = true;
    }
    if (flags & 0x04) {
        chain = 0xff;
    }
    if (!address_from_pubkey(G_context.pk_info.raw_public_key,
                             chain,
                             bounceable,
                             testnet,
                             address,
                             sizeof(address))) {
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }
    base64_encode(address, sizeof(address), g_address, sizeof(g_address));

    // Launch
    g_validate_callback = &ui_action_validate_pubkey;
    ux_flow_init(0, ux_display_pubkey_flow, NULL);

    return 0;
}

// Step with icon and text
UX_STEP_NOCB(ux_display_review_step,
             pnn,
             {
                 &C_icon_eye,
                 "Review",
                 g_operation,
             });
// Step with title/text for amount
UX_STEP_NOCB(ux_display_amount_step,
             bnnn_paging,
             {
                 .title = "Amount",
                 .text = g_amount,
             });
// Step with payload amount
UX_STEP_NOCB(ux_display_payload_step,
             bnnn_paging,
             {
                 .title = "Payload",
                 .text = g_payload,
             });

// FLOW to display transaction information:
// #1 screen : eye icon + "Review Transaction"
// #2 screen : display amount
// #3 screen : display destination address
// #4 screen : approve button
// #5 screen : reject button
UX_FLOW(ux_display_transaction_flow,
        &ux_display_review_step,
        &ux_display_address_step,
        &ux_display_amount_step,
        &ux_display_payload_step,
        &ux_display_approve_step,
        &ux_display_reject_step);

int ui_display_transaction() {
    if (G_context.req_type != CONFIRM_TRANSACTION || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    // Operation
    snprintf(g_operation, sizeof(g_operation), "Transaction");

    // Amount
    if ((G_context.tx_info.transaction.send_mode & 128) != 0) {
        snprintf(g_amount, sizeof(g_amount), "ALL YOUR TONs");
    } else {
        memset(g_amount, 0, sizeof(g_amount));
        char amount[30] = {0};
        if (!format_fpu64(amount,
                          sizeof(amount),
                          G_context.tx_info.transaction.value,
                          EXPONENT_SMALLEST_UNIT)) {
            return io_send_sw(SW_DISPLAY_AMOUNT_FAIL);
        }
        snprintf(g_amount, sizeof(g_amount), "TON %.*s", sizeof(amount), amount);
    }
    PRINTF("Amount: %s\n", g_amount);

    // Address
    uint8_t address[ADDRESS_LEN] = {0};
    address_to_friendly(G_context.tx_info.transaction.to_chain,
                        G_context.tx_info.transaction.to_hash,
                        true,
                        false,
                        address,
                        sizeof(address));
    memset(g_address, 0, sizeof(g_address));
    base64_encode(address, sizeof(address), g_address, sizeof(g_address));

    // Payload
    if (G_context.tx_info.transaction.payload > 0) {
        memset(g_payload, 0, sizeof(g_payload));
        base64_encode(G_context.tx_info.transaction.payload_hash, 32, g_payload, sizeof(g_payload));
    } else {
        snprintf(g_payload, sizeof(g_payload), "Nothing");
    }

    // Start flow
    g_validate_callback = &ui_action_validate_transaction;
    ux_flow_init(0, ux_display_transaction_flow, NULL);

    return 0;
}
