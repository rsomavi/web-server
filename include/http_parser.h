#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#define MAX_ARGUMENTOS 10
#define MAX_URI_LENGTH 100

#include "http_utils.h"

/**
 * @brief Parses an HTTP request from the client connection.
 *
 * @param connfd The file descriptor for the client connection.
 * @param verbo Pointer to store the HTTP method type (GET, POST, etc.).
 * @param archivo Buffer to store the requested file path.
 * @param num_args Pointer to store the number of URL arguments parsed.
 * @param args Array to store the URL argument key-value pairs.
 * @param req_headers Pointer to store the parsed request headers.
 * @param body Buffer to store the request body content.
 * @param verbose Verbosity level.
 * @return 0 on success, or a negative value on error.
 */
int parse_request(int connfd,
				  method_type_t *verbo,
				  char req_line[MAX_URI_LENGTH],
				  char archivo[MAX_URI_LENGTH],
				  int *num_args,
				  char args[MAX_ARGUMENTOS][2][50],
				  headers **req_headers,
				  char body[8192], bool verbose);

/**
 * @brief Extracts and parses URL query arguments from the request path.
 *
 * @param archivo The request path containing query parameters.
 * @param num_args Pointer to store the number of arguments found.
 * @param args Array to store the parsed argument key-value pairs.
 */
void get_url_args(char archivo[MAX_URI_LENGTH],
				  int *num_args,
				  char args[MAX_ARGUMENTOS][2][50], bool verbose);

#endif