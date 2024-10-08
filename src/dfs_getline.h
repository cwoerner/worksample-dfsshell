/*
 * dfs_getline.h
 *
 *      Author: cwoerner
 */

#include <stdio.h>

#ifndef GETLINE_H_
#define GETLINE_H_

int dfs_getline(FILE * f, char *s, int lim);

short whitespace(char c);

char *stripwhite(char *string);

#endif                          /* GETLINE_H_ */
