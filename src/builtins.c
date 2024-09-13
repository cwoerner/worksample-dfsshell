/*
 * builtins.c
 *
 *      Author: cwoerner
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/errno.h>
#include <assert.h>
#include "cmdparse.h"
#include "dfs.h"
#include "dfslist.h"
#include "builtins.h"
#include "path.h"

#define DFS_STR_INIT_ERR int __str_init_errno = errno; \
	errno = 0; \
	fprintf(stderr, "%s\n", strerror(__str_init_errno)); \
	return __str_init_errno;

dfs_command_t builtins[] = {
    { "cd", com_cd, "Change to directory DIR" },
    { "pwd", com_pwd, "Show current working directory" },
    { "rename", com_rename, "Rename a file within the current dfs" },
    { "cat", com_cat,
     "Print the contents of the file to stdout (`cat -text <path>', `cat <path>')"
     },
    { "exit", com_exit, "Quit using dfssh" },
    { "quit", com_exit, "Alias for exit" },
    { "help", com_help, "Display this text" },
    { "?", com_help, "Alias for help" },
    { "ls", com_ls, "List files in current directory" },
    { "dfs", com_dfs,
     "Open a new subshell for the given file system. (`dfs [dfs name]' or `dfs [-ls|-stack|-pop]'"
     },
    { "dfsls", com_dfsls,
     "Show the list of valid dfs (alias for `dfs -ls')" },
    { "dfsstack", com_dfsstack,
     "Show the stack of currently opened dfs sessions (alias for `dfs -stack')"
     },
    { "dfspop", com_dfspop,
     "Alias for exit - exit the current dfs context (alias for `dfs -pop')"
     },
    { (char *) NULL, (builtin_func *) NULL, (char *) NULL }
};

char cmdbuf[CMD_BUFLEN];

int __dfsshell_system(char *cmd)
{
    return system(cmd);
}

int com_rename(char *arg, dfs_shell_t *shell)
{
    snprintf(cmdbuf, CMD_BUFLEN, "rename %s", arg);
    return (__dfsshell_system(cmdbuf));
}

int com_cat(char *arg, dfs_shell_t *shell)
{
    char *arg_text = "text";
    size_t arg_text_len = strlen(arg_text);

    char c;
    char *p = arg;
    while ((c = *p) && isspace(c))
        p++;

    if (0 == strlen(p)) {
        fprintf(stderr, "missing argument(s) to cat: %s\n",
                strerror(EINVAL));
        return EINVAL;
    }

    short dashes = 0;
    short is_text = 0;          // if the argument is just "-", then it's stdin
    for (; (c = *(p + dashes)) && c == '-'; dashes++);  // noop
    if (dashes > 0 && 0 == strncmp((p + dashes), arg_text, arg_text_len)) {
        p += dashes + arg_text_len;
        is_text = 1;
    }

    while ((c = *p) && isspace(c))
        p++;

    if (0 == strlen(p)) {
        fprintf(stderr, "invalid argument(s) to cat (%s): %s\n", arg,
                strerror(EINVAL));
        return EINVAL;
    }

    short free_p = 0;
    if (*p != '/') {
        char *cwd = shell->dfs_stack->head->state->cwd;
        char *tmp = p;
        size_t tmplen = strlen(tmp);
        size_t cwdlen = strlen(cwd);

        DFS_STR_INIT(p, tmplen + cwdlen, cwd, cwdlen, DFS_STR_INIT_ERR);
        strncat(p, "/", 1);
        strncat(p, tmp, tmplen);
    }

    snprintf(cmdbuf, CMD_BUFLEN, "cat %s", p);
    int rc = __dfsshell_system(cmdbuf);

    if (free_p)
        free(p);
    return rc;
}

int com_pwd(char *arg, dfs_shell_t *shell)
{
    printf("%s\n", shell->dfs_stack->head->state->cwd);
    return 0;
}


int com_dfs(char *arg, dfs_shell_t *shell)
{
    int tmperrno;

    if (0 == strncmp(arg, "-ls", 3)) {
        return com_dfsls(NULL, shell);
    }

    if (0 == strncmp(arg, "-stack", 6)) {
        return com_dfsstack(NULL, shell);
    }

    if (0 == strncmp(arg, "-pop", 4)) {
        return com_dfspop(NULL, shell);
    }

    dfs_state_t *cur_state = shell->dfs_stack->head->state;
    dfs_t *dfsp =
        dfs_list_item_find(shell->dfs_list, (dfs_t) { arg, NULL, NULL,
                           NULL, 0
                           }
    );
    if (!dfsp) {
        tmperrno = errno ? errno : EINVAL;
        errno = 0;
        fprintf(stderr, "failed to find dfs: %s\n", strerror(tmperrno));
        return tmperrno;
    }

    int cwdlen = strlen(cur_state->cwd);

    char *cwd;
    DFS_STR_INIT(cwd, cwdlen, cur_state->cwd, cwdlen, DFS_STR_INIT_ERR);

    dfs_state_t *state = dfs_state_create(cwd, dfsp);
    if (NULL == state) {
        tmperrno = errno ? errno : EINVAL;
        errno = 0;
        fprintf(stderr, "failed to create state: %s\n",
                strerror(tmperrno));
        return tmperrno;
    }

    if (!dfs_shell_stack_push(shell->dfs_stack, state)) {
        int tmperrno = errno ? errno : EINVAL;
        errno = 0;
        fprintf(stderr, "failed to push state: %s\n", strerror(tmperrno));
        return tmperrno;
    }

    return 0;
}

int com_dfspop(char *arg, dfs_shell_t *shell)
{
    if (shell->dfs_stack->len == 1) {
        fprintf(stderr, "pop would leave stack empty: %s\n",
                strerror(ERANGE));
        return ERANGE;
    }

    dfs_state_t *oldstate = dfs_shell_stack_pop(shell->dfs_stack);
    if (NULL == oldstate) {
        int tmperrno = errno ? errno : ENOENT;
        errno = 0;
        fprintf(stderr, "failed to pop item off stack: %s\n",
                strerror(tmperrno));
        return tmperrno;
    } else {
        dfs_state_free(oldstate);
        return 0;
    }
}

int com_dfsstack(char *arg, dfs_shell_t *shell)
{
    dfs_shell_stack_item_t *item = shell->dfs_stack->head;
    dfs_t *dfsp;
    struct tm tm_time;
    char *cwd;
    int tmbuflen = 80;
    char tmbuf[tmbuflen];

    int i = 0;
    char *active_dfs = shell->dfs_stack->head->state->dfs->name;
    size_t active_dfs_len = strlen(active_dfs);

    while (item) {
        dfsp = item->state->dfs;
        localtime_r(&(item->state->start_time), &tm_time);
        cwd = item->state->cwd;
        strftime(tmbuf, tmbuflen, "%a %b %d %T %Z %Y", &tm_time);
        printf("%s %d %s %s:%s\n",
               (strncmp(dfsp->name, active_dfs, active_dfs_len) ==
                0 ? "*" : " "), i++, tmbuf, dfsp->name, cwd);
        item = item->next;
    }

    return 0;
}

int com_dfsls(char *arg, dfs_shell_t *shell)
{
    dfs_list_item_t *item = shell->dfs_list->head;

    int i = 0;
    char *active_dfs = shell->dfs_stack->head->state->dfs->name;
    size_t active_dfs_len = strlen(active_dfs);

    while (item) {
        printf("%s %d %s\t%s\t%s\n",
               (strncmp(item->dfs->name, active_dfs, active_dfs_len) ==
                0 ? "*" : " "), i++, item->dfs->name, item->dfs->url,
               item->dfs->desc);
        item = item->next;
    }

    return 0;
}

int com_help(char *arg, dfs_shell_t *shell)
{
    register int i;
    int printed = 0;

    for (i = 0; builtins[i].name; i++) {
        if (!*arg || (strcmp(arg, builtins[i].name) == 0)) {
            printf("%s\t\t%s.\n", builtins[i].name, builtins[i].doc);
            printed++;
        }
    }

    if (!printed) {
        printf("No commands match '%s'.  Possibilities are:\n", arg);

        for (i = 0; builtins[i].name; i++) {
            /* Print in six columns. */
            if (printed == 6) {
                printed = 0;
                puts("");
            }

            printf("%s\t", builtins[i].name);
            printed++;
        }

        if (printed)
            puts("");
    }

    return (0);
}

int com_exit(char *arg, dfs_shell_t *shell)
{
    dfs_shell_stack_t *stack = shell->dfs_stack;
    dfs_state_t *oldstate;
    while (stack->len > 0) {
        oldstate = dfs_shell_stack_pop(stack);
        if (NULL == oldstate) {
            if (!errno)
                errno = ENOENT;
        } else {
            dfs_state_free(oldstate);
        }
    }

    if (errno) {
        int tmperrno = errno;
        errno = 0;
        fprintf(stderr, "failed to clear stack: %s\n", strerror(tmperrno));
        return tmperrno;
    }

    return 0;
}


int com_cd(char *arg, dfs_shell_t *shell)
{
    // TODO: cwd should be a bounded, circular linked-list based stack so that we support "cd -"
    size_t len;
    if (NULL == arg || 0 == (len = strlen(arg))) {
        arg = getenv(ENV_HOME);
        if (NULL == arg || 0 == (len = strlen(arg))) {
            arg = PATHSEP;
            len = strlen(arg);
        }
    }

    dfs_state_t *state = shell->dfs_stack->head->state;
    char *buf;
    DFS_STR_INIT(buf, len, arg, len, DFS_STR_INIT_ERR);
    char *resolved_path = resolve_path(buf, state->cwd);
    free(buf);
    if (NULL == resolved_path) {
        int tmperrno = errno;
        errno = 0;
        fprintf(stderr, "failed to resolve path: %s\n",
                strerror(tmperrno));
        return tmperrno;
    }
    //TODO: check whether resolved_path exists before setting state->cwd

    if (resolved_path != NULL) {
        if (state->cwd != NULL)
            free(state->cwd);
        state->cwd = resolved_path;
    }

    return (0);
}


int com_ls(char *arg, dfs_shell_t *shell)
{
    char *flags[MAX_TOKENS];
    char *args[MAX_TOKENS];

    int rc = 0;
    int arg_count = 0;
    int flag_count = 0;

    if ((rc =
         parse_command_opts(arg, flags, &flag_count, args,
                            &arg_count)) != 0) {
        fprintf(stderr, "Failed to parse the command.\n");
    } else {

        //snprintf(cmdbuf, CMD_BUFLEN, "hadoop fs -fs %s -ls %s",
        //         dfs_stack->head->state->dfs->name, arg);
        if (arg_count == 0) {

            arg = shell->dfs_stack->head->state->cwd;
            if (!arg)
                arg = "/";
            args[arg_count++] = safe_strndup(arg, MAX_TOKEN_LENGTH);
        }

        snprintf(cmdbuf, CMD_BUFLEN, "ls %s %s",
                 join_args(flags, flag_count, ' '), join_args(args,
                                                              arg_count,
                                                              ' '));

        rc = (__dfsshell_system(cmdbuf));
    }

    for (int i = 0; flags[i] != NULL && i < flag_count; i++)
        free(flags[i]);
    for (int i = 0; args[i] != NULL && i < arg_count; i++)
        free(args[i]);
    return rc;
}
