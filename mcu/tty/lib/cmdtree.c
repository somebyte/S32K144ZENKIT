/*
 * ops_btree.c
 *
 *  Created on: Feb 17, 2021
 *      Author: somebyte
 */

#include "../cmdtree.h"
#include <stdlib.h>
#include <string.h>

struct cmdtree
{
  cmdnode_ptr_t  root;
  unsigned int   count;
};

struct cmdnode
{
  char*         name;
  cmdnode_ptr_t left;
  cmdnode_ptr_t right;
  cmdfunc_ptr_t fptr;
};

cmdtree_ptr_t
create_cmdtree (void)
{
  cmdtree_ptr_t tree = (cmdtree_ptr_t) malloc (sizeof (struct cmdtree));

  if (tree == NULL)
    {
      return NULL;
    }

  tree->root  = NULL;
  tree->count = 0;

  return tree;
}

cmdfunc_ptr_t
search_command (const cmdtree_ptr_t tree, const char* cmdname)
{
  if (tree == NULL || cmdname == NULL)
    {
      return NULL;
    }

  cmdnode_ptr_t node = tree->root;

  for(;;)
    {
      if(node == NULL)
        {
          return NULL;
        }

      int cmp = strcmp(node->name, cmdname);

      if (cmp == 0)
        {
          return node->fptr;
        }
      else
      if (cmp > 0)
        {
          node = node->right;
        }
      else
        {
          node = node->left;
        }
    } 
}

cmdnode_ptr_t
insert_command (cmdtree_ptr_t tree, const char* cmdname, cmdfunc_ptr_t fptr)
{
  if (tree == NULL||cmdname == NULL||fptr == NULL)
  {
      return NULL;
  }

  cmdnode_ptr_t *last = &tree->root;
  cmdnode_ptr_t  node =  tree->root;
  size_t       size = strlen (cmdname) + 1;

  for(;;)
    {
      if (node == NULL)
        {
          node = (cmdnode_ptr_t) malloc (sizeof (struct cmdnode));
          node->left  = NULL;
          node->right = NULL;
          node->fptr  = fptr;

          node->name  = (char*) malloc (size);
          if (node->name != NULL)
            {
              memset(node->name, 0, size);
              strcpy(node->name, cmdname);
            }

          *last = node;
          return node;
        }

      int cmp = strcmp(node->name, cmdname);

      if (cmp == 0)
        {
          return node;
        }
      else
      if (cmp > 0)
        {
          last = &node->right;
          node = node->right;
        }
      else
        {
          last = &node->left;
          node = node->left;
        }
    }
}

