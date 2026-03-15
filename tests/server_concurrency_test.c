#include <unistd.h>
#include <stdio.h>
#include "net_lib.h"
#include "concurrency.h"

void test_handler(int connfd, server_config *sc)
{
	// Simplemente enviamos un "ACK!" para confirmar que el pool funciona [2, 6]
	write(connfd, "ACK! Servidor Concurrente funcionando\n", 38);
}

int main()
{
	// 1. Abrimos el servidor (puerto 8080, backlog 10) [6]
	int listen_fd = open_server(8080, 10);
	if (listen_fd < 0)
	{
		printf("hola1");
		return 1;
	}

	// 2. Lanzamos el pool de 5 procesos usando nuestro test_handler [3]
	create_static_process_pool(listen_fd, 5, test_handler, NULL);

	return 0;
}