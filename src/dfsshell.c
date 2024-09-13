/*
 * dfs_shell.c
 *
 *  Created on: Jul 22, 2011
 *      Author: cwoerner
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "dfsshell.h"


dfs_shell_t *dfs_shell_create()
{
    dfs_shell_t *shell = malloc(sizeof(dfs_shell_t));
    if (NULL == shell) {
        errno = ENOMEM;
        goto SHELL;
    }

    shell->prompt = NULL;

    shell->dfs_list = dfs_list_create();
    if (NULL == shell->dfs_list) {
        errno = ENOMEM;
        goto SHELL;
    }
    shell->dfs_stack = dfs_shell_stack_create();
    if (NULL == shell->dfs_stack) {
        errno = ENOMEM;
        goto SHELL;
    }

  SHELL:
    if (errno) {
        dfs_shell_free(shell);
    }
    return shell;
}

void dfs_shell_free(dfs_shell_t *shell)
{
    if (NULL == shell)
        return;

    if (NULL != shell->prompt)
        free(shell->prompt);
    if (NULL != shell->dfs_stack)
        dfs_shell_stack_free(shell->dfs_stack);
    if (NULL != shell->dfs_list)
        dfs_list_free(shell->dfs_list);

    free(shell);
}

char *dfs_shell_mkprompt(dfs_shell_t *shell)
{
    dfs_state_t *cur_state = shell->dfs_stack->head->state;

    char *dfs_cwd_sep = ":";
    char *eoshell = "> ";

    size_t dfsnamelen = strlen(cur_state->dfs->name);
    size_t dfscwdseplen = strlen(dfs_cwd_sep);
    size_t cwdlen = strlen(cur_state->cwd);
    size_t eoshellen = strlen(eoshell);

    size_t bufsize = sizeof(char) *
        (dfsnamelen + dfscwdseplen + cwdlen + eoshellen + 1);

    if (NULL != shell->prompt) {
        shell->prompt = realloc(shell->prompt, bufsize);
    } else {
        shell->prompt = malloc(bufsize);
    }

    if (NULL != shell->prompt) {
        memset(shell->prompt, 0, bufsize);
        strncpy(shell->prompt, cur_state->dfs->name, dfsnamelen);
        strncat(shell->prompt, dfs_cwd_sep, dfscwdseplen);
        strncat(shell->prompt, cur_state->cwd, cwdlen);
        strncat(shell->prompt, eoshell, eoshellen);
    }

    return shell->prompt;
}
