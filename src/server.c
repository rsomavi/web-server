#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "concurrency.h"
#include "conf.h"
#include "net_lib.h"
#include "file_handler.h"
#include "http_parser.h"
#include "http_utils.h"
#include "api_client.h"

/**
 * @brief Logs an HTTP request to console and pipe for file logging.
 *
 * This function formats a log entry with timestamp, request line, HTTP status,
 * and bytes sent. It outputs to both console and the logging pipe.
 *
 * @param req_line The HTTP request line (method and path).
 * @param st The HTTP status code (OK, BAD_REQ, NOT_FOUND).
 * @param bytes_enviados The number of bytes sent in the response.
 * @param pipe Array containing write end of the logging pipe.
 */
void log_request(char req_line[MAX_URI_LENGTH], status_t st, int bytes_enviados, int pipe[2])
{
	char *date;
	int st_num;

	if (st == OK)
		st_num = 200;
	else if (st == BAD_REQ)
		st_num = 400;
	else if (st == NOT_FOUND)
		st_num = 404;

	char log[256];

	parse_datetime(&date, -1);
	sprintf(log, "[%s] \"%s\" %d %d\n", date, req_line, st_num, bytes_enviados);
	printf("%s", log);
	fflush(stdout);

	write(pipe[1], log, strlen(log));

	free(date);
}

/**
 * @brief Receives and processes an HTTP request from a client.
 *
 * This function is the main request handler that parses incoming HTTP requests,
 * determines the appropriate action based on the method and path, and delegates
 * to the appropriate handler (static files, scripts, API, etc.). It also handles
 * error responses and logs all requests.
 *
 * @param connfd The file descriptor for the client connection.
 * @param sc Pointer to the server configuration structure.
 * @param pipe Array containing the pipe descriptors for logging.
 */
void recieve_request(int connfd, server_config *sc, int pipe[2])
{
	char req_line[MAX_URI_LENGTH];
	char archivo[MAX_URI_LENGTH];
	char body[8192];
	method_type_t verbo;
	status_t st = OK;
	headers *req_header = NULL;
	int bytes_mandados = 0;

	int n_args;						  // argc
	char args[MAX_ARGUMENTOS][2][50]; // argv

	if (parse_request(connfd, &verbo, req_line, archivo, &n_args, args,
					  &req_header, body, sc->verbose) == -1)
	{
		// hay que devolver 404 o 400
		st = BAD_REQ;
		bytes_mandados = serve_err(connfd, NULL, BAD_REQ, "mala request por varios motivos", sc->verbose);
		log_request(req_line, st, bytes_mandados, pipe);
		return;
	}

	char path[256];

	sprintf(path, "./%s%s", sc->server_root, archivo);

	if (sc->verbose)
	{
		printf("\tpath despues de saneo: %s\n", path);
		fflush(stdout);
	}

	if (strlen(sc->api_route) && strncmp(archivo, sc->api_route, strlen(sc->api_route)) == 0 && verbo == GET)
		serve_api(connfd, archivo + 4); // interceptar APIs; por ahora solo GET
	else if (strlen(sc->scripts_directory) && strncmp(archivo, sc->scripts_directory, strlen(sc->scripts_directory)) == 0 && (verbo == GET || verbo == HEAD || verbo == POST))
	{
		int len = 0;
		if (req_header != NULL)
			len = req_header->content_length;

		bytes_mandados = execute_file(connfd, path, args, n_args, body, len, &st, sc->verbose);
		if (verbo == POST)
			free(req_header);
	}
	else if (verbo == GET || verbo == HEAD)
	{
		struct stat stt;

		if (stat(path, &stt) == 0 && S_ISDIR(stt.st_mode))
			strcat(path, "/index.html"); // mejor redirijir el cliente mandando 301 con la location nueva pero bueno

		bytes_mandados = serve_file(connfd, path, verbo == GET, &st, sc->verbose);
	}
	else if (strlen(sc->shared_directory) && (verbo == PUT || verbo == DELETE))
	{
		if (strncmp(archivo, sc->shared_directory, strlen(sc->shared_directory)) != 0)
		{
			st = BAD_REQ;
			bytes_mandados = serve_err(connfd, NULL, BAD_REQ, "intentando modificar/borrar archivo fuera del folder compartido", sc->verbose);
			free(req_header);
			log_request(req_line, st, bytes_mandados, pipe);
			return;
		}

		if (verbo == PUT)
		{
			bytes_mandados = modify_file(connfd, path, body, req_header->content_length, &st, sc->verbose);
			free(req_header);
		}
		else if (verbo == DELETE)
			bytes_mandados = delete_file(connfd, path, &st, sc->verbose);
	}
	else if (verbo == OPTIONS)
		bytes_mandados = send_options(connfd, sc->server_signature);
	else if (verbo == OTHER_METHOD)
	{
		st = BAD_REQ;
		bytes_mandados = serve_err(connfd, NULL, BAD_REQ, "metodo/verbo no soportado", sc->verbose);
	}
	else
	{
		st = BAD_REQ;
		bytes_mandados = serve_err(connfd, NULL, BAD_REQ, "request invalida", sc->verbose);
	}

	log_request(req_line, st, bytes_mandados, pipe);
}

/**
 * @brief Initializes and starts the HTTP server.
 *
 * This function prints the server configuration to the console, opens the
 * server socket on the configured port, and creates the process pool to
 * handle incoming connections.
 *
 * @param sc Pointer to the server configuration structure.
 */
void setup_server(server_config *sc)
{
	printf("abriendo servidor en %d\n\tcon backlog de %d\n\tcon max_clients de %d\n\tcon el root en %s\n\tgenerando logs en %s\n", sc->port, sc->backlog, sc->max_clients, sc->server_root, sc->logs_directory);
	if (strlen(sc->shared_directory))
		printf("\tcon archivos compartidos activados en %s\n", sc->shared_directory);
	if (strlen(sc->scripts_directory))
		printf("\tcon scripts activados en %s\n", sc->scripts_directory);
	if (strlen(sc->api_route))
		printf("\tcon api activada en %s\n", sc->api_route);
	printf("\n%s\n\n\n", sc->server_signature);
	fflush(stdout);

	int listen_fd = open_server(sc->port, sc->backlog);

	create_static_process_pool(listen_fd, sc->max_clients, recieve_request, sc);
}

int main(int argc, char *argv[])
{
	char *config_file = NULL;
	if (argc > 1)
		config_file = argv[1];

	server_config *sc = configurate(config_file);

	// error en el conf o no se encuentra el conf file
	if (sc == NULL)
	{
		printf("no se ha encontrado el config file '%s'", config_file);
		return 1;
	}

	setup_server(sc);
}