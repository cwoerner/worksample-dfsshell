/*
 * builtins.h
 *
 *  Created on: Jul 22, 2011
 *      Author: cwoerner
 */

#include "dfsshell.h"
#include "path.h"

#ifndef BUILTINS_H_
#define BUILTINS_H_



#ifndef CMD_BUFLEN
#define CMD_BUFLEN 4096
#endif

#ifndef ENV_HOME
#define ENV_HOME "HOME"
#endif


typedef int builtin_func(char *, dfs_shell_t *);

typedef struct _dfs_command_t {
    char *name;
    builtin_func *func;
    char *doc;
} dfs_command_t;

int com_cd(char *, dfs_shell_t * shell),
com_pwd(char *arg, dfs_shell_t * shell),
com_rename(char *arg, dfs_shell_t * shell),
com_cat(char *arg, dfs_shell_t * shell),
com_exit(char *, dfs_shell_t * shell),
com_help(char *, dfs_shell_t * shell),
com_ls(char *, dfs_shell_t * shell),
com_dfs(char *, dfs_shell_t * shell),
com_dfsls(char *, dfs_shell_t * shell),
com_dfsstack(char *, dfs_shell_t * shell),
com_dfspop(char *, dfs_shell_t * shell);

#endif                          /* BUILTINS_H_ */
