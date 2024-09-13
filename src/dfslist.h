/*
 * dfslist.h
 *
 *      Author: cwoerner
 */

#include "dfs.h"


#ifndef DFSLIST_H_
#define DFSLIST_H_

#ifndef DFS_YAML
#define DFS_YAML "/qc/qmr/conf/dfs.yaml"
#endif

#ifndef DFS_YAML_LINEBUF_MAX
#define DFS_YAML_LINEBUF_MAX 256
#endif


#define SQUOTE (char)39
#define DQUOTE (char)34

typedef struct _dfs_list_item_t {
    struct _dfs_list_item_t *next;
    struct _dfs_list_item_t *prev;
    dfs_t *dfs;
} dfs_list_item_t;

typedef struct _dfs_list_t {
    dfs_list_item_t *head;
    dfs_list_item_t *tail;
    int len;
} dfs_list_t;

void dfs_list_free(dfs_list_t * list);
dfs_list_t *dfs_list_create();
int dfs_list_add(dfs_list_t * list, dfs_t * data);
dfs_t *dfs_list_item_find(dfs_list_t * list, dfs_t proto);
dfs_list_t *dfs_list_read(dfs_list_t * list);

#endif                          /* DFSLIST_H_ */
