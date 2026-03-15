#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "http_utils.h"
#include "conf.h"

/**
 * @brief Converts a content_type_t enum to its HTTP string representation.
 *
 * @param type The content type enum value to convert.
 * @return A string representation of the content type (e.g., "text/html").
 */
char *content_type_to_string(content_type_t type)
{
	switch (type)
	{
	case TEXT_PLAIN:
		return "text/plain";
	case TEXT_HTML:
		return "text/html";
	case TEXT_CSS:
		return "text/css";
	case IMAGE_GIF:
		return "image/gif";
	case IMAGE_JPEG:
		return "image/jpeg";
	case IMAGE_PNG:
		return "image/png";
	case IMAGE_ICO:
		return "image/x-icon";
	case VIDEO_MPEG:
		return "video/mpeg";
	case APP_MSWORD:
		return "application/msword";
	case APP_PDF:
		return "application/pdf";
	case APP_JSON:
		return "application/json";
	case EXECUTABLE:
		// habria que ejecutar el archivo y luego retornar la salida (como text/plain imagino)
		return "text/plain";
	default:
		return "";
	}
}

/**
 * @brief Converts a string to the corresponding content_type_t enum value.
 *
 * @param string The content type string (e.g., "text/html").
 * @return The corresponding content_type_t enum value, or OTHER_CONTENT_TYPE if unknown.
 */
content_type_t string_to_content_type(char *string)
{
	if (strcmp("text/plain", string) == 0)
		return TEXT_PLAIN;
	else if (strcmp("text/html", string) == 0)
		return TEXT_HTML;
	else if (strcmp("text/css", string) == 0)
		return TEXT_CSS;
	else if (strcmp("image/gif", string) == 0)
		return IMAGE_GIF;
	else if (strcmp("image/jpeg", string) == 0)
		return IMAGE_JPEG;
	else if (strcmp("image/png", string) == 0)
		return IMAGE_PNG;
	else if (strcmp("image/x-icon", string) == 0)
		return IMAGE_ICO;
	else if (strcmp("video/mpeg", string) == 0)
		return VIDEO_MPEG;
	else if (strcmp("application/msword", string) == 0)
		return APP_MSWORD;
	else if (strcmp("application/pdf", string) == 0)
		return APP_PDF;
	else if (strcmp("application/json", string) == 0)
		return APP_JSON;
	else if (strcmp("application/x-www-form-urlencoded", string) == 0)
		return APP_FORM_REQ;
	else
		return OTHER_CONTENT_TYPE;
}

int set_content_type(char path[256], headers *h)
{
	// busca el ultimo punto para comprobar la extension
	char *ext = strrchr(path, '.');

	if (!ext || ext == path)
		return -1;
	ext++; // quita el punto y deja solo la extension

	if (strcmp(ext, "txt") == 0)
		h->content_type = TEXT_PLAIN;
	else if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0)
		h->content_type = TEXT_HTML;
	else if (strcmp(ext, "css") == 0)
		h->content_type = TEXT_CSS;
	else if (strcmp(ext, "gif") == 0)
		h->content_type = IMAGE_GIF;
	else if (strcmp(ext, "jpeg") == 0 || strcmp(ext, "jpg") == 0)
		h->content_type = IMAGE_JPEG;
	else if (strcmp(ext, "png") == 0)
		h->content_type = IMAGE_PNG;
	else if (strcmp(ext, "ico") == 0)
		h->content_type = IMAGE_ICO;
	else if (strcmp(ext, "mpeg") == 0 || strcmp(ext, "mpg") == 0)
		h->content_type = VIDEO_MPEG;
	else if (strcmp(ext, "doc") == 0 || strcmp(ext, "docx") == 0)
		h->content_type = APP_MSWORD;
	else if (strcmp(ext, "pdf") == 0)
		h->content_type = APP_PDF;
	else if (strcmp(ext, "json") == 0)
		h->content_type = APP_JSON;
	else if (strcmp(ext, "py") == 0 || strcmp(ext, "php") == 0)
		h->content_type = EXECUTABLE;
	else
		h->content_type = OTHER_CONTENT_TYPE;

	return 0;
}

void parse_datetime(char **date, time_t t)
{
	if (t == -1)
		t = time(NULL);

	// Usamos gmtime porque HTTP siempre usa la hora media de Greenwich (GMT/UTC)
	struct tm *tm_info = gmtime(&t);

	// %a: Día abreviado
	// %d: Día del mes
	// %b: Mes abreviado
	// %Y: Año
	// %H:%M:%S: Hora completa
	// mirar esto porque falla con sizeof(date) vv TODO
	*date = malloc(sizeof(char) * 50);
	strftime(*date, sizeof(char) * 50, "%a, %d %b %Y %H:%M:%S GMT", tm_info);
}

headers *empty_headers()
{
	headers *h = malloc(sizeof(headers));
	if (h == NULL)
		return NULL;

	h->date = NULL;
	h->server = NULL;
	h->last_modified = NULL;
	h->content_length = 0;
	h->content_type = OTHER_CONTENT_TYPE;
	h->status = BAD_REQ;

	return h;
}

headers *default_headers()
{
	// TODO: malloc y eso gestionarlo como es debido
	headers *h = empty_headers();
	if (h == NULL)
		return NULL;

	parse_datetime(&(h->date), -1);

	h->server = malloc(sizeof(char) * 256);
	strcpy(h->server, DEFAULT_SIGNATURE);

	h->status = OK; // por defecto se considera OK

	return h;
}

// por si se necesita en un futuro pero realmente esto se usa para conseguir el contenttype y length de los post (entre otros) requests

int send_headers(int connfd, headers *h, bool verbose)
{
	char temp[256];
	char header[4096] = "HTTP/1.1 ";

	// printf("DEBUG send_headers: status=%d len=%ld\n",
	// 	   h->status, h->content_length);

	switch (h->status)
	{
	case OK:
		strcat(header, "200 OK\r\n");
		break;
	case BAD_REQ:
		strcat(header, "400 Bad Request\r\n");
		break;
	case NOT_FOUND:
		strcat(header, "404 Not Found\r\n");
		break;
	}

	sprintf(temp, "Server: %s\r\n", h->server);
	strcat(header, temp);

	sprintf(temp, "Date: %s\r\n", h->date);
	strcat(header, temp);

	sprintf(temp, "Content-Type: %s\r\n", content_type_to_string(h->content_type));
	strcat(header, temp);

	sprintf(temp, "Content-Length: %ld\r\n", h->content_length);
	strcat(header, temp);

	sprintf(temp, "Last-Modified: %s\r\n", h->last_modified);
	strcat(header, temp);

	strcat(header, "\r\n");

	if (verbose)
	{
		printf("\tse va a responder con la siguiente cabecera:\n\n%s", header);
		fflush(stdout);
	}

	return send(connfd, header, strlen(header), 0);
}

int send_options(int connfd, char *server)
{
	char resp[256];
	sprintf(resp, "HTTP/1.1 200 OK\r\nAllow: GET, POST, PUT, DELETE, OPTIONS\r\nContent-Length: 0\r\nServer: %s\r\n\r\n", server);

	return send(connfd, resp, strlen(resp), 0);
}

int send_response(int connfd, FILE *f, headers *h, bool verbose)
{
	char buffer[4096];
	size_t bytes_leidos;
	int total_enviado = 0;

	// if (f == NULL)
	// 	printf("no abre el archivo\n");

	int bytes_cabecera = send_headers(connfd, h, verbose);

	// se va leyendo y enviando el archivo por chunks de 4kb
	while ((bytes_leidos = fread(buffer, 1, sizeof(buffer), f)) > 0)
	{
		total_enviado += bytes_leidos;
		send(connfd, buffer, bytes_leidos, 0);
	}

	// printf("bytes enviados: %d\n\n\n", total_enviado);
	fclose(f);
	return bytes_cabecera + total_enviado;
}
