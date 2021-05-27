/*
 * fw.h
 *
 *  Created on: Apr 7, 2021
 *      Author: somebyte
 */
#ifndef BOARD_RPC_FW_H_
#define BOARD_RPC_FW_H_

#include <stdint.h>

extern uint32_t APP_BEGIN_ADDRESS;

void download_fw ();
void jump_to_fw  ();

#endif /* BOARD_RPC_FW_H_ */
