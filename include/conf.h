#ifndef CONF_H
#define CONF_H

#include <stdbool.h>

#define DEFAULT_ROOT "www/"
#define DEFAULT_LOG_DIR "logs/"
#define DEFAULT_SHARED_DIR ""
#define DEFAULT_SCRIPTS_DIR ""
#define DEFAULT_API_ROUTE ""

#define DEFAULT_PORT 8080
#define DEFAULT_BACKLOG 10
#define DEFAULT_MAX_CLIENTS 10

#define DEFAULT_VERBOSE false

#define DEFAULT_SIGNATURE "webserver german ruben - 1.0"

// typedef enum
// {
// 	POOL_STATIC,
// 	POOL_DYNAMIC
// } conn_type_t;

typedef struct
{
	// conn_type_t conn_type;
	int max_clients;
	int backlog;
	char *server_root;
	char *server_signature;
	int port;
	bool verbose;
	char *logs_directory;
	char *shared_directory;
	char *scripts_directory;
	char *api_route;
} server_config;

/**
 * @brief Configures and initializes the server with default or custom settings.
 *
 * @return A pointer to a dynamically allocated server_config structure, or NULL on failure.
 */
server_config *configurate();

/**
 * @brief Frees the memory allocated for the configuration file.
 */
void free_config(server_config *sv);

#endif