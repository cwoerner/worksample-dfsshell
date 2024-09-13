/*
 * dfs_shell.h
 *
 *      Author: cwoerner
 */

#include "dfs.h"
#include "dfslist.h"

#ifndef DFS_SHELL_H_
#define DFS_SHELL_H_

#ifndef PROMPT_BUFSIZE
#define PROMPT_BUFSIZE 4096
#endif


typedef struct _dfs_shell_t {
    dfs_shell_stack_t *dfs_stack;
    dfs_list_t *dfs_list;
    char *prompt;
} dfs_shell_t;


void dfs_shell_free(dfs_shell_t * shell);

dfs_shell_t *dfs_shell_create();

char *dfs_shell_mkprompt(dfs_shell_t * shell);

#endif                          /* DFS_SHELL_H_ */
