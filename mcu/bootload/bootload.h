/*
 * bootload.h
 *
 *  Created on: Apr 21, 2021
 *      Author: somebyte
 */

#ifndef BOOTLOAD_BOOTLOAD_H_
#define BOOTLOAD_BOOTLOAD_H_

extern int need_jump_to_fw;

int bootloadmain();

int upload(const void*);
int jump  (const void*);

#endif /* BOOTLOAD_BOOTLOAD_H_ */
