/*
 * dfs.h
 *
 *  Created on: Jul 21, 2011
 *      Author: cwoerner
 */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/errno.h>

#define DFS_STR_INIT(b, l, v, vl, handle) \
	assert(l >= vl); \
	size_t _str_alloc_x = sizeof(char) * (l + 1); \
	b = malloc(_str_alloc_x); \
	if (NULL == b) { \
		errno = ENOMEM; \
		handle; \
	} \
	memset(b, 0, l); \
	strncpy(b, v, vl);


#ifndef FSROOT
#define FSROOT "/"
#ifdef FSROOT_LEN
#undef FSROOT_LEN
#endif
#endif

#ifndef FSROOT_LEN
#define FSROOT_LEN strlen(FSROOT)
#endif

#ifndef DFS_H_
#define DFS_H_

typedef struct _dfs_t {
    char *name;
    char *url;
    char *desc;
    char *ver;
    short worm;
} dfs_t;


typedef struct _dfs_state_t {
    dfs_t *dfs;
    char *cwd;
    time_t start_time;
} dfs_state_t;

typedef struct _dfs_shell_stack_item_t {
    dfs_state_t *state;
    struct _dfs_shell_stack_item_t *next;
} dfs_shell_stack_item_t;

typedef struct _dfs_shell_stack_t {
    struct _dfs_shell_stack_item_t *head;
    int len;
} dfs_shell_stack_t;

void dfs_free(dfs_t * dfs);

void dfs_state_free(dfs_state_t * state);

dfs_state_t *dfs_state_create(char *cwd, dfs_t * dfs);

void dfs_shell_stack_free(dfs_shell_stack_t * stack);

dfs_shell_stack_t *dfs_shell_stack_create();

int dfs_shell_stack_push(dfs_shell_stack_t * stack, dfs_state_t * state);

dfs_state_t *dfs_shell_stack_pop(dfs_shell_stack_t * stack);

#endif                          /* DFS_H_ */
