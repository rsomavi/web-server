/**
 * @file concurrency.h
 * @author [Your Name/Partner Name]
 * @brief Concurrency management module for a Pre-forked Web Server.
 *
 * This module implements a static process pool architecture. It spawns a
 * fixed number of worker processes (children) that handle incoming
 * connections concurrently, improving performance and system stability [3, 4].
 */

#ifndef CONCURRENCY_H
#define CONCURRENCY_H

#include "conf.h"
#include <sys/types.h>

/**
 * @brief Callback function type for processing HTTP requests.
 *
 * This pointer allows the concurrency module to remain generic. It will
 * execute any task assigned by the main server, such as parsing HTTP
 * or serving static files [5, 6].
 *
 * @param connfd The file descriptor for the accepted client connection.
 * @param server_config pointer to the server configurations.
 */
typedef void (*request_handler_t)(int, server_config *, int[2]);

/**
 * @brief Creates a static pool of child processes (Pre-forked Server).
 *
 * Spawns a set number of worker processes at startup. The parent process
 * remains in a waiting state, while the children perform all the
 * networking tasks [3, 7]. This model prevents DoS attacks by
 * limiting the maximum number of concurrent clients [8].
 *
 * @param listenfd The server's listening socket (already bound and listening).
 * @param nchildren The number of processes to spawn (from 'max_clients' in config).
 * @param handler The function that will process each client request.
 * @param sc Pointer to the server configuration structure.
 * @return void
 */
void create_static_process_pool(int listenfd, int nchildren, request_handler_t handler, server_config *sc);

#endif /* CONCURRENCY_H */