/*
 * rpc.h
 *
 *  Created on: Feb 20, 2021
 *      Author: somebyte
 */

#ifndef RPC_RPC_H_
#define RPC_RPC_H_

#include "cmdtree.h"

typedef int (*extra_ptr_t)(void);

/* ROOT of command tree */
extern cmdtree_ptr_t proctree_ptr;

cmdtree_ptr_t proctree_init ();
int           callproc      (const char* instruction);

/* virtual methods */
extern extra_ptr_t extra_proctree;
extern extra_ptr_t extra_help;

#endif /* RPC_RPC_H_ */
