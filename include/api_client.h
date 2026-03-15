#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <stdio.h>

/**
 * @brief Serves API requests by processing the endpoint and sending responses.
 *
 * @param connfd The file descriptor for the client connection.
 * @param endpoint The API endpoint path to process.
 */
void serve_api(int connfd, const char *endpoint);

#endif
