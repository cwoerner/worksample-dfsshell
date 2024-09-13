/*
 * dfs.c
 *
 *      Author: cwoerner
 */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>
#include "dfs.h"

void dfs_free(dfs_t *dfs)
{
    if (NULL == dfs)
        return;

    if (NULL != dfs->name)
        free(dfs->name);

    if (NULL != dfs->url)
        free(dfs->url);

    if (NULL != dfs->desc)
        free(dfs->desc);

    if (NULL != dfs->ver)
        free(dfs->ver);

    free(dfs);
}

dfs_state_t *dfs_state_create(char *cwd, dfs_t *dfsp)
{
    dfs_state_t *state = malloc(sizeof(dfs_state_t));
    if (NULL == state) {
        errno = ENOMEM;
        goto STATE;
    }
    state->start_time = time(0);
    state->cwd = cwd;
    state->dfs = dfsp;

  STATE:
    return state;
}


void dfs_state_free(dfs_state_t *state)
{
    free(state->cwd);
    free(state);
}

dfs_shell_stack_t *dfs_shell_stack_create()
{
    dfs_shell_stack_t *stack = malloc(sizeof(dfs_shell_stack_t));
    if (NULL == stack) {
        errno = ENOMEM;
        goto STACK;
    }
    stack->head = NULL;
    stack->len = 0;

  STACK:
    return stack;
}

int dfs_shell_stack_push(dfs_shell_stack_t *stack, dfs_state_t *state)
{
    dfs_shell_stack_item_t *item = malloc(sizeof(dfs_shell_stack_item_t));
    if (NULL == item) {
        errno = ENOMEM;
        goto PUSH;
    }

    item->state = state;
    item->next = stack->head;
    stack->head = item;
    stack->len++;

  PUSH:
    if (errno) {
        return 0;
    }

    return stack->len;
}

dfs_state_t *dfs_shell_stack_pop(dfs_shell_stack_t *stack)
{
    if (0 == stack->len) {
        errno = ERANGE;
        return NULL;
    }

    dfs_shell_stack_item_t *popped = stack->head;
    dfs_state_t *state = popped->state;
    stack->head = popped->next;
    stack->len--;
    free(popped);
    return state;               // was allocated by caller
}

void dfs_shell_stack_free(dfs_shell_stack_t *stack)
{
    if (NULL == stack) {
        errno = EINVAL;
        return;
    }

    dfs_shell_stack_item_t *item = stack->head;
    dfs_shell_stack_item_t *tmp;
    while (NULL != item) {
        tmp = item;
        dfs_state_free(item->state);
        item = tmp->next;
        free(tmp);
    }

    free(stack);
}
