/*
 * cmdsrvr.h
 *
 *      Author: cwoerner
 */


#include <sys/types.h>

#ifndef CMDSRVR_H_
#define CMDSRVR_H_


#ifndef SRVR_CMD
#define SRVR_CMD "java -Xmx128 -server com.dfsshell.server.Server"
#endif

pid_t cmdsrvr_spawn();

int cmdsrvr_await(pid_t server_pid);

#endif                          /* CMDSRVR_H_ */
