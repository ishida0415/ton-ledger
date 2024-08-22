#ifndef _IO_H_
#define _IO_H_

#include <stdint.h>

#include "ux.h"

#include "types.h"

void io_seproxyhal_display(const bagl_element_t *element);

/**
 * Function for IO event.
 *
 * @brief io callback called when an interrupt based channel has received data
 * to be processed.
 *
 * @return 1 if success, 0 otherwise.
 *
 */
uint8_t io_event(uint8_t channel);

uint16_t io_exchange_al(uint8_t channel, uint16_t tx_len);

/**
 * Function to receive APDU command.
 *
 * @brief receive APDU command in G_io_apdu_buffer and update output_len.
 *
 * @return positive integer if success, -1 otherwise.
 *
 */
int recv_command(void);

/**
 * Function to send back APDU response (response data + status word).
 *
 * @brief send back APDU response (response data + status word) by filling
 * G_io_apdu_buffer with resp and sw.
 *
 * @param resp buffer with APDU reponse data and its length.
 * @param sw status word of APDU response (2 bytes).
 *
 * @return positive integer if success, -1 otherwise.
 *
 */
int send_response(const response_t *resp, uint16_t sw);

/**
 * Function to send back APDU response (only status word).
 *
 * @brief send back APDU response (only status word) by filling
 * G_io_apdu_buffer with sw.
 *
 * @param sw status word of APDU response (2 bytes).
 *
 * @return positive integer if success, -1 otherwise.
 *
 */
int send_sw(uint16_t sw);

#endif  // _IO_H_
