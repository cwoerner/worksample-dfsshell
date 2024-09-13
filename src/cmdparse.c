#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "cmdparse.h"

int is_flag(const char *token)
{
    return token[0] == '-' && strlen(token) > 1;
}

// Function to safely duplicate a string up to a specified length and handle allocation errors
char *safe_strndup(const char *str, size_t n)
{
    char *dup = strndup(str, n);
    if (dup == NULL) {
        perror("strndup failed");
        exit(EXIT_FAILURE);
    }
    return dup;
}

// Function to parse a command string
int parse_command(const char *command_str, char **base_cmd, char **flags,
                  char **args, int *arg_count)
{
    char *command_copy, *token, *saveptr;
    int flag_count = 0;
    *arg_count = 0;

    // Make a copy of the command string, limit length to MAX_TOKEN_LENGTH
    command_copy = safe_strndup(command_str, MAX_TOKEN_LENGTH);

    // Tokenize the first part as the base command
    token = strtok_r(command_copy, " ", &saveptr);
    if (token == NULL) {
        fprintf(stderr, "Error: No base command found.\n");
        free(command_copy);
        return -1;
    }

    *base_cmd = safe_strndup(token, MAX_TOKEN_LENGTH);  // Store the base command

    // Loop through the rest of the tokens
    while ((token = strtok_r(NULL, " ", &saveptr)) != NULL) {
        if (is_flag(token)) {
            flags[flag_count++] = safe_strndup(token, MAX_TOKEN_LENGTH);        // Store flags
        } else {
            args[(*arg_count)++] = safe_strndup(token, MAX_TOKEN_LENGTH);       // Store arguments
        }

        // Ensure we don't exceed MAX_TOKENS limit
        if (flag_count >= MAX_TOKENS || *arg_count >= MAX_TOKENS) {
            fprintf(stderr, "Error: Too many flags or arguments.\n");
            free(command_copy);
            return -1;
        }
    }

    // Mark the end of the flags and arguments lists
    flags[flag_count] = NULL;
    args[*arg_count] = NULL;

    free(command_copy);
    return 0;
}

// Function to parse command options
int parse_command_opts(const char *opts_str,
                       char **flags, int *flag_count,
                       char **args, int *arg_count)
{
    char *opts_copy, *token, *saveptr;
    *flag_count = 0;
    *arg_count = 0;

    // Make a copy of the command string, limit length to MAX_TOKEN_LENGTH
    opts_copy = safe_strndup(opts_str, MAX_TOKEN_LENGTH);

    // Tokenize the first part as the base command
    token = strtok_r(opts_copy, " ", &saveptr);
    if (token == NULL) {
        goto OUT;
    }
    // Loop through the rest of the tokens
    while ((token = strtok_r(NULL, " ", &saveptr)) != NULL) {
        if (is_flag(token)) {
            flags[(*flag_count)++] = safe_strndup(token, MAX_TOKEN_LENGTH);     // Store flags
        } else {
            args[(*arg_count)++] = safe_strndup(token, MAX_TOKEN_LENGTH);       // Store arguments
        }

        // Ensure we don't exceed MAX_TOKENS limit
        if (*flag_count >= MAX_TOKENS || *arg_count >= MAX_TOKENS) {
            fprintf(stderr, "Error: Too many flags or arguments.\n");
            free(opts_copy);
            return -1;
        }
    }

    // Mark the end of the flags and arguments lists
    flags[*flag_count] = NULL;
    args[*arg_count] = NULL;

  OUT:
    free(opts_copy);
    return 0;
}

// Function to join an array of strings with a given separator
char *join_args(char *args[], int num_args, char separator)
{
    if (num_args <= 0) {
        // If no arguments, return an empty string
        char *empty_string = malloc(1);
        if (empty_string == NULL) {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }
        empty_string[0] = '\0';
        return empty_string;
    }
    // Calculate the total length required
    size_t total_length = 0;
    for (int i = 0; i < num_args; i++) {
        total_length += strlen(args[i]);
        // Add space for separators, but not after the last string
        if (i < num_args - 1) {
            total_length += 1;  // For the separator
        }
    }

    // Allocate memory for the result string
    char *result = malloc(total_length + 1);    // +1 for null terminator
    if (result == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    // Concatenate strings with separator
    result[0] = '\0';           // Start with an empty string
    for (int i = 0; i < num_args; i++) {
        strcat(result, args[i]);
        // Add the separator if not the last argument
        if (i < num_args - 1) {
            size_t len = strlen(result);
            result[len] = separator;
            result[len + 1] = '\0';
        }
    }

    return result;
}
