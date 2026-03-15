#include "net_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int open_server(int puerto, int backlog)
{
	int listen_fd;
	struct sockaddr_in serv_addr;

	// 1. socket(): Crear el extremo de comunicación
	// Se usa AF_INET para IPv4 y SOCK_STREAM para TCP [4, 5]
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0)
	{
		// Si el SO no puede crear el socket, retornamos error inmediatamente
		return -1;
	}

	// 1.1. si el puerto ya esta en uso (o "cooldown") (porque hemos cerrado y abierto el servidor muy rapido) que podamos reutilizar el puerto
	int opt = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		return -1;

	// 2. Preparar la estructura de dirección
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY; // Escucha en todas las interfaces (0.0.0.0) [6]
	serv_addr.sin_port = htons(puerto);		// Conversión a orden de red (Big Endian) [4]

	// 3. bind(): Registrar la dirección y puerto en el sistema
	if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		// IMPORTANTE: Si bind falla (ej. puerto ocupado), hay que cerrar el socket
		// para que Valgrind no detecte fugas de descriptores de fichero [3]
		close(listen_fd);
		return -1;
	}

	// 4. listen(): Establecer la cola de peticiones pendientes
	// El 'backlog' limita cuántos clientes pueden esperar en cola [7]
	if (listen(listen_fd, backlog) < 0)
	{
		// Si falla, cerramos el recurso antes de salir
		close(listen_fd);
		return -1;
	}

	// Retornamos el descriptor listo para ser usado por accept() más adelante
	return listen_fd;
}

int accept_connection(int listen_fd)
{
	struct sockaddr_in client_addr; // Estructura para la info del cliente [5]
	socklen_t client_len = sizeof(client_addr);
	int connection_fd;

	// La función se bloquea aquí hasta que llega un cliente [1, 4]
	connection_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
	if (connection_fd < 0)
	{
		return -1; // Error al aceptar la conexión
	}

	return connection_fd; // Retornamos el nuevo socket específico para el cliente [1]
}