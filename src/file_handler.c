#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#include "file_handler.h"
#include "conf.h"
#include "scripts.h"

int serve_err(int connfd, headers *h, status_t st, char *msg, bool verbose)
{
	int bytes_sent = 0;
	if (h == NULL)
	{
		h = default_headers();
		if (h == NULL)
			return -1;
	}

	h->content_type = TEXT_PLAIN;
	h->content_length = strlen(msg);
	h->status = st;

	parse_datetime(&(h->last_modified), -1);

	bytes_sent = send_headers(connfd, h, verbose);
	bytes_sent += send(connfd, msg, strlen(msg), 0);

	free(h);

	return bytes_sent;
}

int execute_file(int connfd, char path[256], char args[MAX_ARGUMENTOS][2][50], int n_args, char body[8192], int len_body, status_t *st, bool verbose)
{
	headers *h = default_headers();
	int bytes_sent = 0;

	if (h == NULL)
		return -1;

	if (set_content_type(path, h) == -1)
	{
		*st = BAD_REQ;
		return serve_err(connfd, h, BAD_REQ, "intentando acceder a archivo sin extension", verbose);
	}

	if (h->content_type != EXECUTABLE)
	{
		*st = BAD_REQ;
		return serve_err(connfd, h, BAD_REQ, "necesita ser ejecutable", verbose);
	}

	struct stat stt;
	if (stat(path, &stt) != 0)
	{
		*st = NOT_FOUND;
		return serve_err(connfd, h, NOT_FOUND, "archivo no encontrado", verbose);
	}

	// hay que ser mas estrictos
	if (strstr(path, "/scripts/") == NULL)
	{
		*st = BAD_REQ;
		return serve_err(connfd, h, BAD_REQ, "ejecutable no se encuentra en /scripts/", verbose); // no deberia de entrar aqui ya
	}

	parse_datetime(&(h->last_modified), stt.st_mtime);
	FILE *file = run_script(path, &(h->content_length), args, n_args, body, len_body);

	bytes_sent = send_response(connfd, file, h, verbose);

	free(h);
	return bytes_sent;
}

// si es archivo no valido sirve 400, si no encuentra el archivo 404, sino intenta servirlo
int serve_file(int connfd, char path[256], bool contents, status_t *st, bool verbose)
{
	int bytes_sent = 0;

	headers *h = default_headers();
	if (h == NULL)
		return -1;

	if (set_content_type(path, h) == -1)
	{
		*st = BAD_REQ;
		return serve_err(connfd, h, BAD_REQ, "intentando acceder a archivo sin extension", verbose);
	}

	if (h->content_type == OTHER_CONTENT_TYPE || h->content_type == EXECUTABLE)
	{
		*st = BAD_REQ;
		return serve_err(connfd, h, BAD_REQ, "intentando acceder a archivo con extension no soportado", verbose);
	}

	// se usa stat para comprobar si existe el archivo, y si existe coger la fecha de mod y tamaño
	struct stat stt;
	if (stat(path, &stt) != 0)
	{
		*st = NOT_FOUND;
		return serve_err(connfd, h, NOT_FOUND, "archivo no encontrado", verbose);
	}

	h->content_length = stt.st_size;
	parse_datetime(&(h->last_modified), stt.st_mtime);

	// la peticion es HEAD, solo devuelve cabeceras
	if (!contents)
		bytes_sent = send_headers(connfd, h, verbose);
	else
	{
		FILE *file = fopen(path, "rb");

		// ya pasa la cabecera a bytes, y lee el archivo y empieza a mandar por socket
		bytes_sent = send_response(connfd, file, h, verbose);
	}

	free(h);
	return bytes_sent;
}

int modify_file(int connfd, char path[256], char body[1024], int content_length, status_t *st, bool verbose)
{
	int bytes_sent = 0;
	headers *h = default_headers();

	if (h == NULL)
		return -1;

	if (set_content_type(path, h) == -1)
	{
		*st = BAD_REQ;
		return serve_err(connfd, h, BAD_REQ, "intentando acceder a archivo sin extension", verbose);
	}

	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
	{
		*st = NOT_FOUND;
		return serve_err(connfd, h, NOT_FOUND, "no se pudo abrir archivo", verbose);
	}

	if (write(fd, body, content_length) < 0)
	{
		free(h);
		close(fd);
		return -1;
	}

	close(fd);
	h->status = OK;
	h->content_length = 0;
	parse_datetime(&(h->last_modified), -1);

	bytes_sent = send_headers(connfd, h, verbose);

	free(h);
	return bytes_sent;
}

int delete_file(int connfd, char path[256], status_t *st, bool verbose)
{
	int bytes_sent = 0;
	headers *h = default_headers();
	if (h == NULL)
		return -1;

	if (set_content_type(path, h) == -1)
	{
		*st = BAD_REQ;
		return serve_err(connfd, h, BAD_REQ, "intentando acceder a archivo sin extension", verbose);
	}

	struct stat stt;
	if (stat(path, &stt) != 0)
	{
		*st = NOT_FOUND;
		return serve_err(connfd, h, NOT_FOUND, "archivo no encontrado", verbose);
	}

	if (remove(path) != 0)
	{
		*st = BAD_REQ;
		return serve_err(connfd, h, BAD_REQ, "no se pudo eliminar el archivo", verbose);
	}

	h->status = OK;
	h->content_length = 0;
	bytes_sent = send_headers(connfd, h, verbose);

	free(h);
	return bytes_sent;
}
