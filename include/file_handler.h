#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <stdbool.h>
#include "http_utils.h"
#include "http_parser.h"

/**
 * @brief Serves a static file to the client over the connection.
 *
 * @param connfd The file descriptor for the client connection.
 * @param path The file path to serve.
 * @param contents If true, includes file contents in the response; otherwise only headers.
 * @return 0 on success, or a negative value on error.
 */
int serve_file(int connfd, char path[256], bool contents, status_t *st, bool verbose);

/**
 * @brief Executes a script file and serves its output to the client.
 *
 * @param connfd The file descriptor for the client connection.
 * @param path The path to the executable script file.
 * @return 0 on success, or a negative value on error.
 */
int execute_file(int connfd, char path[256], char args[MAX_ARGUMENTOS][2][50], int n_args, char body[8192], int len_body, status_t *st, bool verbose);

/**
 * @brief Modifies an existing file by writing the provided body content to it.
 *
 * @param connfd The file descriptor for the client connection.
 * @param path The path to the file to modify.
 * @param body The new content to write to the file.
 * @param content_lenght The length of the body content.
 * @return 0 on success, or a negative value on error.
 */
int modify_file(int connfd, char path[256], char body[1024], int content_length, status_t *st, bool verbose);

/**
 * @brief Deletes a file from the filesystem.
 *
 * @param connfd The file descriptor for the client connection.
 * @param path The path to the file to delete.
 * @return 0 on success, or a negative value on error.
 */
int delete_file(int connfd, char path[256], status_t *st, bool verbose);

/**
 * @brief Sends an error response to the client with the specified status and message.
 *
 * @param connfd The file descriptor for the client connection.
 * @param h Pointer to the headers structure to use for the response.
 * @param st The HTTP status code to send.
 * @param msg The error message to include in the response body.
 * @return 0 on success, or a negative value on error.
 */
int serve_err(int connfd, headers *h, status_t st, char *msg, bool verbose);

#endif