/*
 * dfslist.c
 *
 *  Created on: Jul 21, 2011
 *      Author: cwoerner
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <ctype.h>
#include "dfslist.h"
#include "dfs.h"
#include "dfs_getline.h"


void dfs_list_free(dfs_list_t *list)
{
    if (NULL == list)
        return;

    dfs_list_item_t *item = list->head;
    dfs_list_item_t *tmp;
    while (NULL != item) {
        dfs_free(item->dfs);
        tmp = item->next;
        free(item);
        item = tmp;
    }

    free(list);
}

dfs_list_t *dfs_list_create()
{
    dfs_list_t *l = malloc(sizeof(dfs_list_t));
    if (NULL == l) {
        errno = ENOMEM;
        return NULL;
    }
    l->head = NULL;
    l->tail = NULL;
    l->len = 0;
    return l;
}

int dfs_list_add(dfs_list_t *list, dfs_t *data)
{
    dfs_list_item_t *item = malloc(sizeof(dfs_list_item_t));
    if (NULL == item) {
        errno = ENOMEM;
        return 0;
    }

    memset(item, 0, sizeof(dfs_list_item_t));
    item->dfs = data;
    if (!list->head) {
        list->head = item;
    }
    item->next = NULL;
    item->prev = list->tail;
    if (list->tail) {
        list->tail->next = item;
    }
    list->tail = item;
    list->len++;
    return list->len;
}

dfs_t *dfs_list_item_find(dfs_list_t *list, dfs_t proto)
{
    dfs_t *ptr = NULL;
    dfs_list_item_t *item = list->head;
    size_t len;
    size_t curlen = 0;
    while (item != NULL) {
        if (0 == strcmp(item->dfs->name, proto.name)) {
            len = strlen(item->dfs->name);
            if (NULL == ptr || len > curlen) {
                // if no match, or this match is longer then use this
                ptr = item->dfs;
                curlen = len;
            }
        }
        item = item->next;
    }

    return ptr;
}

dfs_list_t *dfs_list_read(dfs_list_t *list)
{
    FILE *fh = fopen(DFS_YAML, "r");
    if (NULL == fh) {
        size_t buflen = 256;
        char msg[(int) buflen];
        snprintf(msg, buflen - 1, "failed to open '%s'", DFS_YAML);
        perror(msg);
        return NULL;
    }

    int len, wslen, toklen, vallen;
    char c;
    char linebuf[DFS_YAML_LINEBUF_MAX];
    char *tokptr, *valptr;
    dfs_t *cur_dfs = NULL;
    char *val = NULL;
    while ((len = dfs_getline(fh, linebuf, DFS_YAML_LINEBUF_MAX)) > 0) {
        if (linebuf[0] == '#')
            continue;

        if (!isspace(linebuf[0])) {
            if (cur_dfs != NULL) {
                // add it to the list of known ones..
                dfs_list_add(list, cur_dfs);
                cur_dfs = NULL;
            }
            // section start
            for (toklen = 0;
                 linebuf[toklen] != ':' && linebuf[toklen] != '\0';
                 toklen++);
            linebuf[toklen] = '\0';
            cur_dfs = malloc(sizeof(dfs_t));
            if (NULL == cur_dfs) {
                errno = ENOMEM;
                break;
            }
            toklen++;
            cur_dfs->name = malloc(sizeof(char) * toklen);
            if (NULL == cur_dfs->name) {
                errno = ENOMEM;
                break;
            }
            strncpy(cur_dfs->name, linebuf, toklen);
        } else {
            // attrib
            if (NULL != cur_dfs) {
                // find first non-ws char
                for (wslen = 0; isspace(linebuf[wslen]); wslen++);
                // find sep char ':'
                tokptr = linebuf + wslen;
                for (toklen = 0;
                     (c = *(tokptr + toklen)) != ':' && c != '\0'
                     && toklen + wslen <= len; toklen++);
                *(tokptr + toklen) = '\0';
                vallen = len - toklen - wslen - 1;      // length of the rest of the data
                val = NULL;
                if (vallen > 0) {
                    valptr = tokptr + toklen + 1;

                    // remove leading spaces, single/double quotes
                    while ((c = *valptr) == SQUOTE || c == DQUOTE
                           || isspace(c))
                        valptr++, vallen--;

                    // remove trailing spaces, single/double quotes
                    while ((c = *(valptr + (vallen - 1))) == SQUOTE
                           || c == DQUOTE || isspace(c)) {
                        *(valptr + (--vallen)) = '\0';  // null pad
                    }

                    val = malloc(sizeof(char) * vallen + 1);
                    if (NULL == val) {
                        errno = ENOMEM;
                        break;
                    }
                    strncpy(val, valptr, vallen);
                }
                if (0 == strncmp(tokptr, "url", toklen)) {
                    cur_dfs->url = val;
                } else if (0 == strncmp(tokptr, "desc", toklen)) {
                    cur_dfs->desc = val;
                } else if (0 == strncmp(tokptr, "ver", toklen)) {
                    cur_dfs->ver = val;
                } else if (0 == strncmp(tokptr, "worm", toklen)) {
                    cur_dfs->worm = (short) strtol(val, NULL, 10);
                }
            }
        }
    }

    if (errno) {
        if (NULL != cur_dfs) {
            free(cur_dfs);
            cur_dfs = NULL;
        }
    } else if (NULL != cur_dfs) {
        // add it to the list of known ones..
        dfs_list_add(list, cur_dfs);
    }

    fclose(fh);
    if (errno) {
        if (NULL != list)
            dfs_list_free(list);
        return NULL;
    }

    return list;
}
