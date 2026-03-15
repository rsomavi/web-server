#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "net_lib.h"

int main()
{
	int port = 8080;
	int backlog = 5;
	int listen_fd, conn_fd;

	// 1. Inicializamos el servidor escuchante [5, 6]
	listen_fd = open_server(port, backlog);
	if (listen_fd < 0)
	{
		return 1; // El error debe gestionarse con GDB, no con printf [7]
	}

	// 2. Entramos en el bucle del servidor iterativo [3, 8]
	while (1)
	{
		// Aceptamos la conexión entrante (bloqueante) [5, 9]
		conn_fd = accept_connection(listen_fd);
		if (conn_fd >= 0)
		{
			// Enviamos una respuesta simple para confirmar la conexión [1, 3]
			char *msg = "ACK! Connection established.\n";
			write(conn_fd, msg, strlen(msg));

			// Cerramos la conexión específica del cliente [3, 5]
			close(conn_fd);
		}
	}

	close(listen_fd);
	return 0;
}
