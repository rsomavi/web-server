#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <ctype.h>
#include <stdbool.h>

#include "http_parser.h"

/**
 * @brief Parses URL-encoded arguments from a string.
 *
 * This function extracts key-value pairs from a URL query string or form data.
 * It validates each argument to prevent injection attacks by checking for
 * allowed characters (alphanumeric, dots, dashes, underscores).
 *
 * @param str The string containing URL-encoded arguments (key1=value1&key2=value2).
 * @param num_args Pointer to store the number of arguments found.
 * @param args Array to store the parsed argument key-value pairs.
 * @param verbose If true, prints debug information about parsed arguments.
 */
void parse_args_from_string(char *str, int *num_args, char args[MAX_ARGUMENTOS][2][50], bool verbose)
{
	char *pareja; // clave y valor
	char *igual;

	if (num_args == NULL)
		*num_args = 0;

	if ((pareja = strtok(str, "&")) && (*num_args < MAX_ARGUMENTOS))
		do
		{
			igual = strchr(pareja, '=');

			// si no encuentra un igual, pasa al siguiente argumento
			if (!igual)
				continue;

			// si el igual esta al final o al principio (argumento= o =valor), esta mal formado y se pasa al siguiente arg
			if (*(igual + 1) == '\0' || igual == pareja)
				continue;

			*igual = '\0';

			// comprueba que no haya caracteres raros
			// asi no se puede hacer algo asi: `scripts/test.py?a3=;cat&a2=/etc/passwd`
			bool valid = true;
			for (int i = 0; (igual + 1)[i] != '\0'; i++)
				if (!isalnum(igual[i + 1]) && igual[i + 1] != '.' && igual[i + 1] != '-' && igual[i + 1] != '_')
				{
					valid = false;
					break;
				}

			if (!valid)
				continue;

			// 49 porque es el maximo (+ \0) de longitud de clave/valor
			strncpy(args[*num_args][0], pareja, 49); // guarda la clave
			strncpy(args[*num_args][1], igual + 1, 49);

			if (verbose)
			{
				if (*num_args == 0)
					printf("\tse ha encontrado argumentos:\n");

				printf("\t\t%s: %s\n", pareja, igual + 1);
				fflush(stdout);
			}

			(*num_args)++;
		} while ((pareja = strtok(NULL, "&")) && (*num_args < MAX_ARGUMENTOS));
}

void get_url_args(char archivo[MAX_URI_LENGTH], int *num_args, char args[MAX_ARGUMENTOS][2][50], bool verbose)
{

	archivo = strtok(archivo, "?"); // limpia el nombre del archivo

	parse_args_from_string(archivo + strlen(archivo) + 1, num_args, args, verbose);
}

/**
 * @brief Sanitizes a file path to prevent directory traversal attacks.
 *
 * This function processes a file path to remove dangerous elements like ".."
 * that could be used to access files outside the intended directory. It uses
 * a stack-based approach to safely reconstruct the path, ensuring that
 * navigation attempts above the root are blocked.
 *
 * @param path The file path to sanitize. This is modified in place.
 */
void sanitize_path(char *path)
{
	char *src = path;
	char *dst = path;
	char *stack[100]; // Punteros a los inicios de cada segmento
	int top = 0;

	// 1. Manejo inicial: Si empieza por "./", saltamos el "."
	if (src[0] == '.' && src[1] == '/')
		src += 2;
	// Si empieza por "/", lo mantenemos
	if (*src == '/')
	{
		*dst++ = '/';
		src++;
	}

	char *token = strtok(src, "/");
	while (token != NULL)
	{
		if (strcmp(token, ".") == 0)
		{
			// Ignorar: "./" no cambia la ruta
		}
		else if (strcmp(token, "..") == 0)
		{
			// ".." sube un nivel: retrocedemos el stack
			if (top > 0)
			{
				top--;
			}
			else
			{
				// Intento de salir por encima de la raíz
				// Por seguridad, forzamos que se quede en la raíz
				top = 0;
			}
		}
		else if (strlen(token) > 0)
		{
			// Segmento válido: guardarlo en el stack
			stack[top++] = token;
		}
		token = strtok(NULL, "/");
	}

	// 2. Reconstruir la ruta sana
	char *write_ptr = dst;
	for (int i = 0; i < top; i++)
	{
		int len = strlen(stack[i]);
		memmove(write_ptr, stack[i], len);
		write_ptr += len;
		if (i < top - 1)
		{
			*write_ptr++ = '/';
		}
	}
	*write_ptr = '\0'; // Fin de cadena

	// Si el resultado quedó vacío, es la raíz
	// if (dst == write_ptr && path[0] != '/')
	// {
	// 	strcpy(path, ".");
	// }
}

/**
 * @brief Parses HTTP headers from the request and populates the headers structure.
 *
 * This function extracts HTTP headers from the request buffer, specifically
 * looking for Content-Type and Content-Length headers. It creates a new
 * headers structure and fills in the parsed values.
 *
 * @param req_headers Pointer to store the newly allocated headers structure.
 * @param line Pointer to the current position in the request buffer.
 *
 * @return Pointer to the position in the buffer after the headers, or NULL on error.
 */
char *parse_headers(headers **req_headers, char *line)
{
	char content_type[50];
	long content_length = 0;

	*req_headers = empty_headers();
	if (*req_headers == NULL)
		return NULL;

	while (line && *line != '\n' && *line != '\0')
	{
		char *next = strchr(line, '\n');
		if (!next)
			break;

		*next = '\0';

		// if (strncmp(line, "Host:", 5) == 0)
		//     sscanf(line, "Host: %99[^\r]", host); // no necesitamos

		// printf("\t%s\n", line);
		// fflush(stdout);

		if (strncmp(line, "Content-Type:", 13) == 0)
		{
			sscanf(line, "Content-Type: %[^\r]", content_type);
			(*req_headers)->content_type = string_to_content_type(content_type);
		}

		if (strncmp(line, "Content-Length:", 15) == 0)
		{
			sscanf(line, "Content-Length: %ld", &content_length);
			(*req_headers)->content_length = content_length;
		}

		line = next + 1;

		if (*line == '\r' || *line == '\n')
		{
			line++;
			break;
		}
	}

	return line;
}

int parse_request(int connfd,
				  method_type_t *verbo,
				  char req_line[MAX_URI_LENGTH],
				  char archivo[MAX_URI_LENGTH],
				  int *num_args,
				  char args[MAX_ARGUMENTOS][2][50],
				  headers **req_headers,
				  char body[8192], bool verbose)
{
	char buff[8192];
	char version[10];
	char verbo_string[10];

	char *request_line;
	char *line;

	int n;

	// TODO: arreglar y hacer recv hasta agotar la cadena vvvv
	// Si el content-line es muy grande y llega en más de un paquete estámos jodidos (Mejora a futuro)
	n = recv(connfd, buff, sizeof(buff) - 1, 0);
	if (n <= 0)
		return -1;

	/* valores por defecto */
	buff[n] = '\0';
	body[0] = '\0';

	request_line = buff;

	// cortar request line
	line = strchr(request_line, '\n');
	if (!line)
		return -1;

	*line = '\0';
	line++;

	// limpiar quitando \r
	if (*(line - 2) == '\r')
		*(line - 2) = '\0';

	// 100 == MAX_URI_LENGTH habria que hacer que se pueda camiar bien;
	// esto es para que no lea de mas y se meta en el body
	sscanf(request_line, "%9s %100s %9s", verbo_string, archivo, version);
	strcpy(req_line, request_line);

	if (verbose)
	{
		printf("\n\nrecibido: \"%s\"\n", request_line);
		fflush(stdout);
	}

	if (strcmp(version, "HTTP/1.1") != 0 && strcmp(version, "HTTP/1.0") != 0)
	{
		if (verbose)
		{
			printf("\t! - peticion mal formada (version HTTP incorrecta; puede que URI muy larga)\n");
			fflush(stdout);
		}

		return -1;
	}

	// se puede hacer una funcion que sea strcmp pero case insensitive
	if (strcmp(verbo_string, "GET") == 0 || strcmp(verbo_string, "get") == 0)
		*verbo = GET;
	else if (strcmp(verbo_string, "HEAD") == 0 || strcmp(verbo_string, "head") == 0)
		*verbo = HEAD;
	else if (strcmp(verbo_string, "POST") == 0 || strcmp(verbo_string, "post") == 0)
		*verbo = POST;
	else if (strcmp(verbo_string, "PUT") == 0 || strcmp(verbo_string, "put") == 0)
		*verbo = PUT;
	else if (strcmp(verbo_string, "DELETE") == 0 || strcmp(verbo_string, "delete") == 0)
		*verbo = DELETE;
	else if (strcmp(verbo_string, "OPTIONS") == 0 || strcmp(verbo_string, "options") == 0)
		*verbo = OPTIONS;
	else
	{
		// printf("verbo '%s' no soportado\n", verbo_string);
		*verbo = OTHER_METHOD;
	}

	if (*verbo == POST || *verbo == PUT)
	{
		// los unicos verbos los cuales importa que parsee las cabeceras y body
		char *fin = strstr(line, "\r\n\r\n");
		if (fin == NULL)
		{
			// significa que las cabeceras son tan largas que no terminan en 8k, simplemente ignoramos la request y no seguimos leyendo por seguridad y optimización
			if (verbose)
			{
				printf("\t! - cabeceras muy largas, ignorando\n");
				fflush(stdout);
			}

			return -1;
		}

		if ((line = parse_headers(req_headers, line)) == NULL)
		{
			if (verbose)
			{
				printf("\t! - fallo al parsear los headers\n");
				fflush(stdout);
			}

			return -1;
		}

		// bytes leidos del cuerpo
		int bytes_leidos = n - (fin + 4 - buff);

		if ((*req_headers)->content_length != bytes_leidos)
		{
			if (verbose)
			{
				printf("\t! - body muy largo, ignorando\n");
				fflush(stdout);
			}

			return -1;
		}

		memcpy(body, fin + 4, (*req_headers)->content_length);
		body[(*req_headers)->content_length] = '\0';

		// el body son args
		if ((*req_headers)->content_type == APP_FORM_REQ)
		{
			parse_args_from_string(body, num_args, args, verbose);
			body[0] = '\0';
			(*req_headers)->content_length = 0;
		}
	}

	// parsea argumentos del url
	get_url_args(archivo, num_args, args, verbose);

	memmove(archivo, archivo + 1, strlen(archivo)); // para quitar el `/` que siempre hay al principio del archivo en vez de poner /%s en el sscanf

	if (verbose)
	{
		printf("\tpath: %s\n", archivo);
		fflush(stdout);
	}

	sanitize_path(archivo);

	return 0;
}