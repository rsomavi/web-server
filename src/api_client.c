#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "api_client.h"
#include "file_handler.h"
#include "conf.h"

/**
 * @brief Executes an external API call using curl and saves the response to a file.
 *
 * This function runs the curl command to fetch data from the specified URL,
 * writes the response to a temporary output file, and returns a file pointer
 * for reading the response.
 *
 * @param url The external API URL to call.
 * @param length Pointer to store the length of the response content.
 *
 * @return FILE pointer to the response file, or NULL on error.
 */
static FILE *call_external_api(const char *url, long *length)
{
	char command[512];
	char buffer[4096];
	size_t n;

	*length = 0;

	FILE *out = fopen("./outputs/api.txt", "wb");
	if (!out)
		return NULL;

	snprintf(command, sizeof(command), "curl -s \"%s\"", url);

	FILE *p = popen(command, "r");
	if (!p)
	{
		fclose(out);
		return NULL;
	}

	while ((n = fread(buffer, 1, sizeof(buffer), p)) > 0)
	{
		*length += n;
		fwrite(buffer, 1, n, out);
	}

	fflush(stdout);

	pclose(p);
	fclose(out);

	return fopen("./outputs/api.txt", "rb");
}

void serve_api(int connfd, const char *endpoint)
{
	headers *h = default_headers();
	if (h == NULL)
		return;

	long len = 0;
	FILE *f = NULL;

	// router de APIs
	if (strcmp(endpoint, "oracion-hoy") == 0)
	{
		f = call_external_api(
			"https://beta.ourmanna.com/api/v1/get/?format=json",
			&len);

		h->content_type = APP_JSON;
	}
	else
	{
		serve_err(connfd, h, NOT_FOUND, "api endpoint not found", false);
		return;
	}

	if (!f)
	{
		serve_err(connfd, h, BAD_REQ, "api error", false);
		return;
	}

	h->content_length = len;
	send_response(connfd, f, h, false);

	free(h);
}
