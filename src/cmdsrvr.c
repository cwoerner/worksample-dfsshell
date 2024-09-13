/*
 * cmdsrvr.c
 *
 *  Created on: Jul 21, 2011
 *      Author: cwoerner
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/wait.h>

#include "cmdsrvr.h"
#include "classpath.h"

pid_t cmdsrvr_spawn()
{
    char *libdir = getenv("DFSSHLIB");
    if (NULL == libdir) {
        libdir = "./lib";
    }

    pid_t jpid = fork();
    if (jpid == 0) {
        int len = 2 << 11;
        char *cpath = malloc(sizeof(char) * len);
        if (NULL == cpath) {
            errno = ENOMEM;
            return 0;
        }
        if (!classpath(cpath, len, libdir)) {
            fprintf(stderr, "failed to make classpath from libdir %s\n",
                    libdir);
        }

        char *const argv[] = { (char *) 0 };
        char *const envp[] = { cpath, (char *) 0 };
        execve(SRVR_CMD, argv, envp);

        // exec failed!
        if (!errno)
            errno = EINVAL;
        perror("failed to execute server program");
        return 0;
    }

    return jpid;
}

int cmdsrvr_await(pid_t server_pid)
{
    int server_stat;
    pid_t exited_pid = waitpid(server_pid, &server_stat, 0);
    if (-1 == exited_pid) {
        char errmsg[1024];
        snprintf(errmsg, 1024, "failed to wait for server process %d",
                 server_pid);
        perror(errmsg);
        return 0;
    }

    if (WIFEXITED(server_stat)) {
        int exit_status = WEXITSTATUS(server_stat);
        if (exit_status != EXIT_SUCCESS) {
            fprintf(stderr,
                    "server process %d exited unsuccessfully (%d)\n",
                    server_pid, exit_status);
        }
    } else {
        fprintf(stderr, "server process %d exited abnormally\n",
                server_pid);
    }

    return 1;
}
