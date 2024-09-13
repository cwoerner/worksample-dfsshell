/*
 * path.h
 *
 *      Author: cwoerner
 */

#ifndef PATH_H_
#define PATH_H_


#ifndef PATHSEP
#define PATHSEP "/"
#endif

#define DOT "."
#define DOTDOT ".."

typedef struct _path_stack_item_t {
    char *path_part;
    size_t len;                 // strlen
    struct _path_stack_item_t *next;
    struct _path_stack_item_t *prev;
} path_stack_item_t;

typedef struct _path_stack_t {
    size_t size;                // num elements
    size_t len;                 // strlen
    size_t seplen;
    char *sep;
    path_stack_item_t *head;
    path_stack_item_t *tail;
} path_stack_t;

char *resolve_path(char *path, char *cwd);
char *canonicalize_path(char *src, char *dst);

path_stack_t *path_stack_create(char *src, char *sep);
size_t path_stack_push(path_stack_t * stack, char *path_part);
char *path_stack_pop(path_stack_t * stack);
char *path_stack_to_path(path_stack_t * stack, char *dst);
void path_stack_free(path_stack_t * stack);


#endif                          /* PATH_H_ */
