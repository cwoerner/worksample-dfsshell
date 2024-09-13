#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "dfs.h"
#include "dfslist.h"
#include "dfs_getline.h"
#include "builtins.h"
#include "dfsshell.h"
#include "cmdsrvr.h"
#include "main.h"

// externs
extern dfs_command_t builtins[];

// static vars
int strict_mode = 0;
char **olongvals = NULL;
dfs_shell_t *shell = NULL;


// functions
int main(int argc, char **argv)
{
    int rc = EXIT_SUCCESS;
    char *cwd = NULL, *dfsname = NULL, *progname = PROGNAME;

    if (olongvals_init() != 0) {
        _SET_RC_ERRNO(rc, errno);
        goto QUIT;
    }

    if (argc > 0) {
        progname = argv[0];
        if (!parse_opts(argc, argv)) {
            _SET_RC_ERRNO(rc, errno);
            goto QUIT;
        }
    }

    cwd = olongvals[opt_p];
    dfsname = olongvals[opt_f];

    shell = dfs_shell_create();
    if (NULL == shell) {
        _SET_RC_ERRNO(rc, errno);
        goto QUIT;
    }

    if (!dfs_list_read(shell->dfs_list)) {
        _SET_RC_ERRNO(rc, errno);
        perror("failed to init dfs list");
        goto QUIT;
    }

    if (NULL == shell->dfs_list->head) {
        _SET_RC_ERRNO(rc, errno);
        perror("no filesystems defined");
        goto QUIT;
    }
    dfs_t *dfs = shell->dfs_list->head->dfs;

    if (NULL != dfsname) {
        if (!(dfs = dfs_list_item_find(shell->dfs_list, (dfs_t) {
                                       dfsname, NULL, NULL, NULL, 0}
              ))) {
            int olderrno = errno;
            size_t buflen = 128;
            char msg[(int) buflen];
            snprintf(msg, buflen - 1, "failed to find dfs '%s'", dfsname);
            if (strict_mode) {
                errno = 0;      // system library calls might have set it since
                _SET_RC_ERRNO(rc, olderrno);
                perror(msg);
                goto QUIT;
            }

            dfs = shell->dfs_list->head->dfs;
            buflen -= strlen(msg);
            strncat(msg, " - using default '", buflen - 1);
            buflen -= strlen(msg);
            strncat(msg, dfs->name, buflen - 1);
            buflen -= strlen(msg);
            strncat(msg, "' ", buflen - 1);
            fputs(msg, stderr);
            puts("");
        }
    }

    dfs_state_t *state = dfs_state_create(cwd, dfs);
    if (NULL == state) {
        _SET_RC_ERRNO(rc, errno);
        perror("failed to create state");
        goto QUIT;
    }
    if (dfs_shell_stack_push(shell->dfs_stack, state) == 0) {
        _SET_RC_ERRNO(rc, errno);
        perror("failed to push state onto stack");
        goto QUIT;
    }

    initialize_readline(progname);

    if (com_cd(state->cwd, shell) != 0) {
        char *msg = "failed to cd to initial working dir";
        if (strict_mode) {
            _SET_RC_ERRNO(rc, errno);
            perror(msg);
            goto QUIT;
        } else {
            fputs(msg, stderr);
        }
    }

    char *line, *s;
    while ((!strict_mode || EXIT_SUCCESS == rc)
           && shell->dfs_stack->len > 0) {
        if (NULL == (line = readline(dfs_shell_mkprompt(shell)))) {
            continue;
        }
        errno = 0;              // why the hell does readline modify errno?
        s = stripwhite(line);
        if (*s) {
            add_history(s);
            rc = execute_line(s);
        }

        free(line);
    }

  QUIT:
    if (errno) {
        perror("problem");
    }
    if (olongvals)
        olongvals_free();
    if (shell)
        dfs_shell_free(shell);

    return rc;
}

static inline int olongvals_init()
{
    olongvals = malloc(sizeof(char *) * opt__len);
    if (NULL == olongvals) {
        return (errno = ENOMEM);
    } else {
        memset(olongvals, 0, sizeof(char *) * opt__len);
    }
    return 0;
}

static inline void olongvals_free()
{
    int i;
    for (i = 0; i < opt__len; i++) {
        if (NULL != olongvals[i]) {
            free(olongvals[i]);
        }
    }
}

char *command_generator(char *text, int state)
{
    static int list_index, len;
    char *name;

    /* If this is a new word to complete, initialize now.  This includes
       saving the length of TEXT for efficiency, and initializing the index
       variable to 0. */
    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    /* Return the next name which partially matches from the command list. */
    while ((name = builtins[list_index].name) != NULL) {
        list_index++;
        if (strncmp(name, text, len) == 0) {
            char *r = malloc(strlen(name) + 1);
            if (NULL == r) {
                errno = ENOMEM;
                // we're kinda deep in the bowels here, so perror to make sure
                // we get a good description of the problem
                perror("failed to allocate memory for command name");
                goto GEN;
            }
            strcpy(r, name);
            return r;
        }
    }

  GEN:
    /* If no names matched, then return NULL. */
    return ((char *) NULL);
}

char **dfssh_completion(const char *text, int start, int end)
{
    char **matches = (char **) NULL;

    /* If this word is at the start of the line, then it is a command
       to complete.  Otherwise it is the name of a file in the current
       directory. */
    if (start == 0)
        matches =
            completion_matches(text, (CPFunction *) command_generator);

    return (matches);
}

void initialize_readline(char *progname)
{
    /* Allow conditional parsing of the ~/.inputrc file. */
    rl_readline_name = progname;

    /* Tell the completer that we want a crack first. */
    rl_attempted_completion_function = (CPPFunction *) dfssh_completion;
}

dfs_command_t *find_command(char *name)
{
    register int i;

    for (i = 0; builtins[i].name; i++) {
        if (strcmp(name, builtins[i].name) == 0) {
            return (&builtins[i]);
        }
    }

    return ((dfs_command_t *) NULL);
}


int execute_line(char *line)
{
    register int i;
    dfs_command_t *command;
    char *word;

    /* Isolate the command word. */
    i = 0;
    while (line[i] && isspace(line[i]))
        i++;
    word = line + i;

    while (line[i] && !isspace(line[i]))
        i++;

    if (line[i])
        line[i++] = '\0';

    if (!(command = find_command(word))) {
        fprintf(stderr, "%s: No such command for dfssh.\n", word);
        return (-1);
    }

    /* Get argument to command, if any. */
    while (isspace(line[i]))
        i++;

    word = line + i;

    return ((*(command->func)) (word, shell));
}


int parse_opts(int argc, char **argv)
{
    int c, optidx;
    size_t bufsize;

    struct option odefs[5] = {
        { "fs", required_argument, 0, 'f' },
        { "pwd", required_argument, 0, 'p' },
        { "strict", no_argument, &strict_mode, 1 },
        // null terminated
        { 0, 0, 0, 0 }
    };

    while (1) {
        optidx = 0;
        c = getopt_long(argc, argv, "f:p:s", odefs, &optidx);
        if (-1 == c)
            break;

        switch (c) {
        case 0:
            if (odefs[optidx].flag != 0)
                break;
            _ASSIGN_OPT(bufsize, optarg, olongvals, optidx);
            break;
        case 'f':
            _ASSIGN_OPT(bufsize, optarg, olongvals, opt_f);
            break;
        case 'p':
            _ASSIGN_OPT(bufsize, optarg, olongvals, opt_p);
            break;
        case 's':
            strict_mode = 1;
            break;
        default:
            errno = EINVAL;
            char *msg =
                "failed to parse command line - '%c' is not a valid option";
            char buf[strlen(msg) + 1];
            sprintf(buf, msg, c);
            perror(buf);
            return 0;
        }
    }

    return 1;
}
