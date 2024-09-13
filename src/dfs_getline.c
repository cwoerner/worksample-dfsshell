/*
 * dfs_getline.c
 *
 *      Author: cwoerner
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int dfs_getline(FILE *f, char *s, int lim)
{
    int c;
    register int i;
    for (i = 0; i < lim - 1 && (c = fgetc(f)) != EOF && c != '\n'; i++)
        *(s + i) = c;

    if (i == lim - 1 && c != EOF) {
        fprintf(stderr, "buffer overflow in dfs_getline\n");
        return 0;
    }

    *(s + i) = '\0';
    return i;
}

char *stripwhite(char *string)
{
    register char *s, *t;

    for (s = string; isspace(*s); s++);

    if (*s == 0)
        return (s);

    t = s + strlen(s) - 1;
    while (t > s && isspace(*t))
        t--;
    *++t = '\0';

    return s;
}
