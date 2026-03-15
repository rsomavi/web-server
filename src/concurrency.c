#include "concurrency.h"
#include "net_lib.h"

#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

/**
 * @brief Internal worker loop for the child process.
 *
 * This function implements the continuous lifecycle of a worker in the
 * pre-forked pool [3]. It performs the following steps in an infinite loop:
 * 1. Blocks on accept_connection() until a client connects [4, 5].
 * 2. Executes the delegated business logic via the 'handler' callback [6].
 * 3. Closes the specific connection socket (connfd) to prevent descriptor leaks [5].
 *
 * @param listenfd The shared master socket descriptor used to listen for connections.
 * @param handler  The generic function pointer containing the task to be performed.
 *
 * @note This function is marked static to encapsulate it within the module [1].
 *       It is designed to never return [3].
 */
static void child_main(int listenfd, request_handler_t handler, server_config *sc, int pipe[2])
{
	for (;;)
	{
		int connfd = accept_connection(listenfd);

		if (connfd >= 0)
		{
			handler(connfd, sc, pipe); // Ejecuta la lógica delegada (ej. enviar ACK!) [6]
			close(connfd);			   // Cierre de la conexión específica [5]
		}
	}
}

server_config *global_config;
int n_procesos;
FILE *log_file;

/**
 * @brief Frees global configuration resources and closes the log file.
 *
 * This function releases the memory allocated for the server configuration
 * and closes the log file if it is open. It is used as a common cleanup
 * routine before process termination.
 */
void cleanup()
{
	free_config(global_config);
	if (log_file)
		fclose(log_file);
}

/**
 * @brief Signal handler for immediate cleanup and process termination.
 *
 * This function is called when a termination signal (e.g., SIGINT) is received.
 * It performs cleanup operations and exits the process immediately without
 * waiting for child processes.
 *
 * @param sig The signal number that triggered this handler.
 */
void cleanup_and_destroy(int sig)
{
	cleanup();
	exit(0);
}

/**
 * @brief Signal handler for cleanup with child process waiting.
 *
 * This function is called when a termination signal (e.g., SIGINT) is received.
 * It performs cleanup operations, waits for all child processes to terminate,
 * and then exits. This ensures proper resource cleanup and zombie process prevention.
 *
 * @param sig The signal number that triggered this handler.
 */
void cleanup_and_wait(int sig)
{
	cleanup();
	for (int i = 0; i < n_procesos; i++)
		wait(NULL);

	printf("\nhijos recogidos, memoria liberada");
	exit(0);
}

void create_static_process_pool(int listenfd, int nchildren, request_handler_t handler, server_config *sc)
{
	int i;
	pid_t childpid;

	global_config = sc;
	n_procesos = nchildren + 1; // los que escuchan requests + el logger

	struct sigaction sa;
	sa.sa_handler = cleanup_and_wait;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	// para que intercepte el ctrl + c y recoja el resto de procesos hijos y libere memoria
	sigaction(SIGINT, &sa, NULL);

	int pipe_log[2];
	pipe(pipe_log);

	if (fork() == 0)
	{
		sa.sa_handler = cleanup_and_destroy;
		sigaction(SIGINT, &sa, NULL);

		close(pipe_log[1]);
		time_t ahora = time(NULL);
		struct tm *t = localtime(&ahora);
		char nombre[100];
		char archivo[200];
		strftime(nombre, 100, "reqs_%Y-%m-%d_%H-%M-%S.log", t);
		sprintf(archivo, "./%s%s", sc->logs_directory, nombre);

		log_file = fopen(archivo, "w");

		char buffer[1024];
		int bytes_leidos;

		while ((bytes_leidos = read(pipe_log[0], buffer, sizeof(buffer) - 1)) > 0)
		{
			buffer[bytes_leidos] = '\0';
			fprintf(log_file, "%s", buffer);
			fflush(log_file);
		}

		return;
	}

	close(pipe_log[0]);

	for (i = 0; i < nchildren; i++)
	{
		childpid = fork(); // Duplicación del proceso [2]

		if (childpid < 0)
		{
			perror("Error en fork");
			exit(1);
		}

		if (childpid == 0)
		{
			sa.sa_handler = cleanup_and_destroy;
			sigaction(SIGINT, &sa, NULL);

			// El hijo entra en su bucle y nunca retorna [2]
			child_main(listenfd, handler, sc, pipe_log);
			exit(0);
		}
	}

	// se crea el logger ahora

	// El padre queda en pausa permanente; los hijos hacen el trabajo [2]
	for (;;)
	{
		pause();
	}
}