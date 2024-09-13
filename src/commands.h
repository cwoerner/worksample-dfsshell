#include <readline/readline.h>
#include <readline/history.h>

/* The names of functions that actually do the manipulation. */
int com_ls(), com_cat(), com_mv(), com_stat(), com_pwd();
int com_rm(), com_help(), com_cd(), com_exit();

/* A structure which contains information on the commands this program
   can understand. */

typedef struct {
    char *name;                 /* User printable name of the function. */
    Function *func;             /* Function to call to do the job. */
    char *doc;                  /* Documentation for this function.  */
} COMMAND;

COMMAND commands[] = {
    { "cat", com_cat, "View the contents of FILE" },
    { "cd", com_cd, "Change to directory DIR" },
    { "exit", com_exit, "Quit using dfsshell" },
    { "help", com_help, "Display this text" },
    { "ls", com_ls, "List files in current directory" },
    { "mv", com_mv, "Rename FILE to NEWNAME" },
    { "pwd", com_pwd, "Print the current working directory" },
    { "rm", com_rm, "Delete FILE" },
    { "stat", com_stat, "Print out statistics on FILE" },
    { (char *) NULL, (Function *) NULL, (char *) NULL }
};
