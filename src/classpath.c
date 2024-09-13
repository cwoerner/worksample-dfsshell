/*
 * classpath.c
 *
 *  Created on: Jul 21, 2011
 *      Author: cwoerner
 */

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>

#include "classpath.h"

char *const jarext = ".jar";

char *classpath(char *classpath, int len, char *const libdir)
{
    DIR *dirp = opendir(libdir);
    if (NULL == dirp) {
        char msg[1024];
        snprintf(msg, 1024, "failed to open %s", libdir);
        perror(msg);
        return NULL;
    }
    // child
    char *ptr;
    int jarext_len = strlen(jarext);
    strncat(classpath, "CLASSPATH=", len);
    strncat(classpath, libdir, len);
    strncat(classpath, ":", len);

    struct dirent *dirent;
    while ((dirent = readdir(dirp)) != NULL) {
        ptr = strstr(dirent->d_name, jarext);
        if (NULL != ptr) {
            if (strlen(ptr) == jarext_len) {
                strncat(classpath, dirent->d_name, len);
                strncat(classpath, ":", len);
            }
        }
    }


    if (closedir(dirp) != 0) {
        char errmsg[1024];
        snprintf(errmsg, 1024, "failed to closedir %s", libdir);
        perror(errmsg);
        return NULL;
    }

    return classpath;
}
