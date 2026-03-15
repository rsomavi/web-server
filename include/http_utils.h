#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include <stdio.h>
#include <stdbool.h>
#include <time.h>

typedef enum
{
	TEXT_PLAIN,	  // .txt
	TEXT_HTML,	  // .html, .htm
	TEXT_CSS,	  // .css
	IMAGE_GIF,	  // .gif
	IMAGE_JPEG,	  // .jpeg, .jpg
	IMAGE_PNG,	  // .png
	IMAGE_ICO,	  // .ico
	VIDEO_MPEG,	  // .mpeg, .mpg
	APP_MSWORD,	  // .doc, .docx
	APP_PDF,	  // .pdf
	EXECUTABLE,	  // .py, .php
	APP_JSON,	  // .json
	APP_FORM_REQ, // los argumentos recibidos por post en scripts
	OTHER_CONTENT_TYPE
} content_type_t;

typedef enum
{
	GET,
	POST,
	OPTIONS,
	PUT,
	DELETE,
	HEAD,
	OTHER_METHOD
} method_type_t;

typedef enum
{
	OK,
	BAD_REQ,
	NOT_FOUND
} status_t;

typedef struct
{
	char *date;					 // fecha actual (?)
	char *server;				 // nombre y version del server (sc->server_signature)
	char *last_modified;		 // ultima fecha de modificacion del archivo
	long content_length;		 // longitud del archivo en bytes
	content_type_t content_type; // el tipo del archivo
	status_t status;			 // el status de la response (200, 400, 404)
} headers;

/**
 * @brief Creates an empty headers structure with all fields initialized to NULL/zero.
 *
 * @return A pointer to a newly allocated headers structure, or NULL on allocation failure.
 */
headers *empty_headers();

/**
 * @brief Creates a headers structure with default values for common fields.
 *
 * @return A pointer to a newly allocated headers structure with default values, or NULL on failure.
 */
headers *default_headers();

/**
 * @brief Parses a timestamp and formats it as an HTTP-compliant date string.
 *
 * @param date Pointer to store the allocated date string.
 * @param t The time_t timestamp to parse.
 */
void parse_datetime(char **date, time_t t);

/**
 * @brief Determines and sets the content type based on the file path extension.
 *
 * @param path The file path to analyze.
 * @param h The headers structure to update with the content type.
 * @return 0 on success, or a negative value if the content type could not be determined.
 */
int set_content_type(char path[256], headers *h);

/**
 * @brief Converts a content_type_t enum to its string representation.
 *
 * @param type The content type enum value to convert.
 * @return A string representation of the content type (e.g., "text/html").
 */
char *content_type_to_string(content_type_t type);

/**
 * @brief Converts a string to the corresponding content_type_t enum value.
 *
 * @param string The content type string (e.g., "text/html").
 * @return The corresponding content_type_t enum value, or OTHER_CONTENT_TYPE if unknown.
 */
content_type_t string_to_content_type(char *string);

/**
 * @brief Sends HTTP response headers to the client over the connection.
 *
 * @param connfd The file descriptor for the client connection.
 * @param h The headers structure containing the header values to send.

 * @return The number of bytes sent
 */
int send_headers(int connfd, headers *h, bool verbose);

/**
 * @brief Sends a complete HTTP response including headers and file contents.
 *
 * @param connfd The file descriptor for the client connection.
 * @param f The file pointer to read content from.
 * @param h The headers structure containing response metadata.

 * @return The number of bytes sent
 */
int send_response(int connfd, FILE *f, headers *h, bool verbose);

/**
 * @brief Sends an HTTP 200 OK response with allowed methods for CORS.
 *
 * @param connfd The file descriptor for the client connection.
 * @param server the server name defined in the config file
 * @return The number of bytes sent
 */
int send_options(int connfd, char *server);

#endif