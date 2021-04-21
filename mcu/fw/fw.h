/*
 * fw.h
 *
 *  Created on: Apr 7, 2021
 *      Author: somebyte
 */
#ifndef BOARD_RPC_FW_H_
#define BOARD_RPC_FW_H_

#include <stdint.h>

#define ERR_OK  0x41
#define ERR_CRC 0x45

extern uint32_t APP_BEGIN_ADDRESS;

void download_fw ();
void jump_to_fw  ();

#endif /* BOARD_RPC_FW_H_ */
