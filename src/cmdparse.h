#ifndef MAX_TOKENS
#define MAX_TOKENS 255
#endif

#ifndef MAX_TOKEN_LENGTH
#define MAX_TOKEN_LENGTH 4096
#endif

int is_flag(const char *token);
char *safe_strndup(const char *str, size_t n);
int parse_command(const char *command_str, char **base_cmd, char **flags, char **args, int *arg_count);
int parse_command_opts(const char *opts_str, char **flags, int *flag_count, char **args, int *arg_count);
char *join_args(char *args[], int num_args, char separator);
