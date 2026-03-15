#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include "conf.h"

/**
 * @brief Creates a server configuration structure with default values.
 *
 * This function allocates memory for a server_config structure and initializes
 * all fields with their default values as defined in the configuration header.
 *
 * @return Pointer to a newly allocated server_config structure with default values.
 */
server_config *default_config()
{
	server_config *sc = malloc(sizeof(server_config));

	sc->port = DEFAULT_PORT;

	sc->server_root = malloc(sizeof(char) * 100);
	strcpy(sc->server_root, DEFAULT_ROOT);

	sc->logs_directory = malloc(sizeof(char) * 100);
	strcpy(sc->logs_directory, DEFAULT_LOG_DIR);

	sc->shared_directory = malloc(sizeof(char) * 100);
	strcpy(sc->shared_directory, DEFAULT_SHARED_DIR);

	sc->scripts_directory = malloc(sizeof(char) * 100);
	strcpy(sc->scripts_directory, DEFAULT_SHARED_DIR);

	sc->api_route = malloc(sizeof(char) * 100);
	strcpy(sc->api_route, DEFAULT_API_ROUTE);

	sc->server_signature = malloc(sizeof(char) * 225);
	strcpy(sc->server_signature, DEFAULT_SIGNATURE);

	sc->backlog = DEFAULT_BACKLOG;

	sc->max_clients = DEFAULT_MAX_CLIENTS;

	sc->verbose = DEFAULT_VERBOSE;

	return sc; // no retornaba sc wow
}

void free_config(server_config *sc)
{
	free(sc->server_root);
	free(sc->logs_directory);
	free(sc->shared_directory);
	free(sc->scripts_directory);
	free(sc->api_route);

	free(sc->server_signature);

	free(sc);
}

/**
 * @brief Sets a configuration value based on the provided key.
 *
 * This function parses a key-value pair from the configuration file and
 * updates the corresponding field in the server_config structure. It supports
 * various configuration options including port, server root, directories,
 * and logging settings.
 *
 * @param sc Pointer to the server_config structure to update.
 * @param key The configuration key to set (e.g., "port", "server_root").
 * @param value The value to assign to the specified key.
 *
 * @return 0 on success, or -1 if the value is invalid or the key is unknown.
 */
int set_value(server_config *sc, char *key, char *value)
{
	if (strcmp(key, "port") == 0)
	{
		if ((sc->port = atoi(value)) == 0)
			return -1;
	}
	else if (strcmp(key, "server_signature") == 0)
		strcpy(sc->server_signature, value);
	else if (strcmp(key, "server_root") == 0)
		strcpy(sc->server_root, value);
	else if (strcmp(key, "logs_directory") == 0)
		strcpy(sc->logs_directory, value);
	else if (strcmp(key, "shared_directory") == 0)
		strcpy(sc->shared_directory, value);
	else if (strcmp(key, "scripts_directory") == 0)
		strcpy(sc->scripts_directory, value);
	else if (strcmp(key, "api_route") == 0)
		strcpy(sc->api_route, value);
	else if (strcmp(key, "max_clients") == 0)
	{
		if ((sc->max_clients = atoi(value)) == 0)
			return -1;
	}
	else if (strcmp(key, "backlog") == 0)
	{
		if ((sc->backlog = atoi(value)) == 0)
			return -1;
	}
	else if (strcmp(key, "verbose") == 0)
	{
		if (strcmp(value, "true") == 0)
			sc->verbose = true;
	}
	else
		printf("no se entiende la entrada '%s' de la configuracion\n", key);

	fflush(stdout);

	return 0;
}

server_config *configurate(char *path)
{
	server_config *configurations = default_config();
	if (configurations == NULL)
		return NULL;

	if (path == NULL || strlen(path) == 0)
		path = "./server.conf";

	FILE *config_file = fopen(path, "r");
	if (config_file == NULL)
		return NULL;

	char line[256];
	char *key, *value;
	while (fgets(line, sizeof(line), config_file))
	{
		// o es linea vacia o un comentario asique se ignora y se pasa a la siguiente linea
		if (strlen(line) == 1 || line[0] == '#')
		{
			continue;
		}

		key = strtok(line, "=");
		if (!key)
			return NULL; // mal archivo .conf

		// quita trailing space si hay
		if (key[strlen(key) - 1] == ' ')
			key[strlen(key) - 1] = '\0';

		value = strtok(NULL, "\0");

		if (!value)
			return NULL; // mal archivo .conf

		if (value[0] == ' ')
			value = value + 1; // quita espacio al principio si hay

		// quita trailing newline si hay
		if (value[strlen(value) - 1] == '\n')
			value[strlen(value) - 1] = '\0';

		// quita \r si hay
		if (value[strlen(value) - 1] == '\r')
			value[strlen(value) - 1] = '\0';

		// quita trailing space si hay
		if (value[strlen(value) - 1] == ' ')
			value[strlen(value) - 1] = '\0';

		if (set_value(configurations, key, value) != 0)
			return NULL; // se ha parseado algo mal
	}

	return configurations;
}
