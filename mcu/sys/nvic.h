/*
 * nvic.h
 *
 *  Created on: Jul 31, 2020
 *      Author: somebyte
 */

#ifndef NVIC_NVIC_H_
#define NVIC_NVIC_H_

#include <stdint.h>

void NVIC_IRQn_init (uint8_t IRQn, uint8_t priority);

#endif /* NVIC_NVIC_H_ */
