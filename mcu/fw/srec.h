/*
 * srec.h
 *
 *  Created on: Apr 6, 2021
 *      Author: somebyte
 */

#ifndef BOARD_RPC_SREC_H_
#define BOARD_RPC_SREC_H_

#include <stdint.h>

#define MIN_SREC       4
#define MAX_SREC       81
#define MAX_ADDRESS_BP 4
#define MAX_DATA_BP    32
#define MAX_PHSIZE_BP  42

union bootphrase
{
  uint8_t bytecode[MAX_PHSIZE_BP];                                   /* Byte level access to the Phrase */
  struct
  {
       char    type;                                                 /* Type of received record (e.g. S0, S1, S5, S9...) */
       uint8_t size;                                                 /* Phrase size (address + data + checksum)          */
       uint8_t address_size;                                         /* Address, depending on the type of record         */
       uint8_t address[MAX_ADDRESS_BP]__attribute__((aligned (32))); /* it might vary                                    */
       uint8_t data_size;                                            /* Maximum 32 data bytes                            */
       uint8_t data[MAX_DATA_BP]      __attribute__((aligned (32))); /* it might vary too                                */
       uint8_t checksum;                                             /* Checksum of size + address + data                */
   } phrase;
};

typedef union bootphrase bootphrase_t;
typedef bootphrase_t* bootphrase_ptr_t;

#endif /* BOARD_RPC_SREC_H_ */

