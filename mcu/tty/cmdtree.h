/*
 * cmd_btree.h
 *
 *  Created on: Feb 17, 2021
 *      Author: somebyte
 */

#ifndef CMD_CMDTREE_H_
#define CMD_CMDTREE_H_

struct cmdtree;
struct cmdnode;

typedef struct cmdtree cmdtree_t;
typedef struct cmdnode cmdnode_t;
typedef cmdtree_t* cmdtree_ptr_t;
typedef cmdnode_t* cmdnode_ptr_t;

typedef int (*cmdfunc_ptr_t)(const void*);

cmdtree_ptr_t create_cmdtree (void);
cmdfunc_ptr_t search_command (cmdtree_ptr_t root, const char* cmdname);
cmdnode_ptr_t insert_command (cmdtree_ptr_t root, const char* cmdname, cmdfunc_ptr_t fptr);

#endif /* CMD_CMDTREE_H_ */
