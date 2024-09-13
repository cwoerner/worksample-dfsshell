/*
 * main.h
 *
 *      Author: cwoerner
 */

#ifndef MAIN_H_
#define MAIN_H_

// macros
#ifndef MAX_FILENAME_LEN
#define MAX_FILENAME_LEN 4096
#endif

#ifndef PROGNAME
#define PROGNAME "dfssh"
#endif

#ifndef _ASSIGN_OPT
#define _ASSIGN_OPT(b, a, o, i) \
	b = sizeof(char) * strlen(a) + 1; \
	o[i] = malloc(b); \
	if (NULL == o[i]) { errno = ENOMEM; return 0; } \
	strncpy(o[i], a, b - 1);
#endif

#ifndef _SET_RC_ERRNO
#define _SET_RC_ERRNO(r, e) if (!e) { e = EINVAL; } \
		r = e;
#endif

// forward declarations
char *command_generator(char *, int);
char **dfssh_completion(const char *, int, int);
void initialize_readline(char *);
dfs_command_t *find_command(char *);
int execute_line(char *);
int parse_opts(int, char **);
static inline int olongvals_init();
static inline void olongvals_free();

// types
enum opt_index {
    opt_f,
    opt_p,

    // used to indicate how many items in the enum there are
    opt__len
};


#endif                          /* MAIN_H_ */
