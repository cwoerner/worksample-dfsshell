/*
 * path.c
 *
 *  Created on: Jul 24, 2011
 *      Author: cwoerner
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/errno.h>
#include "dfs.h"
#include "path.h"

char *resolve_path(char *path, char *cwd)
{
    char *dst = NULL;
    if (NULL == path) {
        if (!errno)
            errno = ENOENT;
        goto RESOLVE;
    }

    while (isspace(*path))
        path++;

    size_t pathlen = strlen(path);
    size_t seplen = strlen(PATHSEP);

    if (strspn(path, PATHSEP) == seplen) {
        // leading '/'
        dst = canonicalize_path(path, NULL);
    } else {
        // relative path
        size_t cwdlen = strlen(cwd);

        char *tmp;
        DFS_STR_INIT(tmp, cwdlen + seplen + pathlen, cwd, cwdlen,
                     goto RESOLVE);
        if (!strncat(tmp, PATHSEP, seplen)) {
            if (!errno)
                errno = EINVAL;
            goto RESOLVE;
        }
        if (!strncat(tmp, path, pathlen)) {
            if (!errno)
                errno = EINVAL;
            goto RESOLVE;
        }

        dst = canonicalize_path(tmp, NULL);
        free(tmp);
    }

    if (dst && strlen(dst) == 0) {
        if (!strncpy(dst, PATHSEP, seplen)) {
            if (!errno)
                errno = EINVAL;
            goto RESOLVE;
        }
    }

  RESOLVE:
    if (errno) {
        if (NULL != dst) {
            free(dst);
        }
        dst = NULL;
    }

    return dst;
}

// dst should have enough space to hold all of src
char *canonicalize_path(char *src, char *dst)
{
    path_stack_t *stack = path_stack_create(src, PATHSEP);
    if (NULL == stack) {
        if (!errno)
            errno = ENOENT;
        dst = NULL;
        goto CANON;
    }

    dst = path_stack_to_path(stack, dst);

  CANON:
    if (stack)
        path_stack_free(stack);
    return dst;
}

path_stack_t *path_stack_create(char *src /*not freed */ ,
                                char *sep /*not freed */ )
{
    path_stack_t *stack = malloc(sizeof(path_stack_t));
    if (NULL == stack) {
        errno = ENOMEM;
        goto STACK;
    }

    stack->head = stack->tail = NULL;
    stack->len = stack->size = 0;
    if (NULL == sep)
        sep = PATHSEP;
    stack->seplen = strlen(sep);

    DFS_STR_INIT(stack->sep, stack->seplen, sep, stack->seplen,
                 goto STACK);

    char *tok, *brkt, *buf;
    size_t toklen;

    for (tok = strtok_r(src, sep, &brkt);
         tok; tok = strtok_r(NULL, sep, &brkt)) {
        toklen = strlen(tok);
        if (toklen > 0) {
            if (strcmp(tok, DOT) != 0) {
                if (strcmp(tok, DOTDOT) != 0) {
                    if (0 == path_stack_push(stack, tok)) {
                        if (!errno)
                            errno = EINVAL;
                        goto STACK;
                    }

                } else {
                    if (NULL == (buf = path_stack_pop(stack))) {
                        if (!errno)
                            errno = ENOENT;
                        goto STACK;
                    }

                    free(buf);
                }
            }
        }
    }

  STACK:
    if (errno) {
        if (stack) {
            if (stack->sep)
                free(stack->sep);
            free(stack);
        }
        stack = NULL;
    }

    if (NULL != stack) {
        buf = path_stack_to_path(stack, NULL);
        free(buf);
    }

    return stack;
}

size_t path_stack_push(path_stack_t *stack, char *path_part)
{
    size_t size = 0;

    path_stack_item_t *item = malloc(sizeof(path_stack_item_t));
    if (NULL == item) {
        errno = ENOMEM;
        goto PUSH;
    }
    size_t pathlen = strlen(path_part);
    DFS_STR_INIT(item->path_part, pathlen, path_part, pathlen, goto PUSH);

  PUSH:
    if (errno) {
        if (NULL != item) {
            if (NULL != item->path_part) {
                free(item->path_part);
            }
            free(item);
        }
        return 0;
    }

    item->len = pathlen;
    item->next = stack->head;
    item->prev = NULL;
    if (NULL != item->next)
        item->next->prev = item;

    if (NULL == stack->tail)
        stack->tail = item;

    stack->head = item;
    stack->size++;
    stack->len += item->len;
    size = stack->size;


    return size;
}

char *path_stack_pop(path_stack_t *stack)
{
    char *path_part = NULL;

    if (0 == stack->size) {
        if (!errno)
            errno = ENOENT;
        goto POP;
    }

    path_stack_item_t *old_head = stack->head;
    path_part = old_head->path_part;
    stack->head = old_head->next;
    if (NULL != stack->head) {
        stack->head->prev = NULL;
    } else {
        stack->tail = NULL;
    }

    stack->size--;
    stack->len -= old_head->len;
    free(old_head);

  POP:
    return path_part;           // caller should free
}

char *path_stack_to_path(path_stack_t *stack, char *dst)
{
    int seplen = stack->seplen;
    char *sep = stack->sep;
    short didalloc = 0;

    // allocate space for a leading '/', a copy of each path_part string, and
    // a sep string between each part
    if (NULL == dst) {
        // allocate/initialize the string
        DFS_STR_INIT(dst, stack->len + ((stack->size + 1) * seplen), sep,
                     seplen, goto PATH);
        didalloc = 1;
    } else {
        // caller is trying to manage memory, assume they got it right and just
        // copy a sep string to the location pointed at by *dst
        strncpy(dst, sep, seplen);
    }

    path_stack_item_t *item = stack->tail;
    while (NULL != item) {
        strncat(dst, item->path_part, item->len);
        item = item->prev;
        if (NULL != item) {
            strncat(dst, sep, seplen);
        }
    }

  PATH:
    if (errno && didalloc) {
        free(dst);
        dst = NULL;
    }
    return dst;
}

void path_stack_free(path_stack_t *stack)
{
    if (NULL == stack) {
        return;
    }

    char *buf;
    while (stack->size > 0) {
        buf = path_stack_pop(stack);
        if (NULL != buf)
            free(buf);
    }

    if (NULL != stack->sep)
        free(stack->sep);

    free(stack);
}
