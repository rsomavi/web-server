#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "scripts.h"

char *get_run_command(char *ext);

/**
 * @brief Executes a command and saves its output to a file.
 *
 * This function redirects the command output to a temporary file, writes the provided
 * body content to the command's stdin, and returns a file pointer to
 * read the output. It is used to capture script execution results.
 *
 * @param exec The command to execute.
 * @param length Pointer to store the length of the output.
 * @param body The content to write to the command's stdin.
 * @param len_body The length of the body content.
 *
 * @return FILE pointer to the output file, or NULL on error.
 */
FILE *guardar_output(char exec[1024], long *length, char body[8192], int len_body);

/**
 * @brief Returns the appropriate interpreter command for a file extension.
 *
 * This function maps common script file extensions to their corresponding
 * interpreter commands. Currently supports Python (.py) and PHP (.php).
 *
 * @param ext The file extension to look up.
 *
 * @return The interpreter command string (e.g., "python3", "php"), or NULL if unsupported.
 */

FILE *run_script(char path[256], long *length, char args[MAX_ARGUMENTOS][2][50], int n_args, char body[8192], int len_body)
{
	char command[1024];
	char *cmd;
	FILE *f;

	command[0] = '\0';
	*length = 0;

	// char *output_path;
	// sprintf(output_path, "./outputs/%s")
	char *ext = strrchr(path, '.');
	if (!ext || ext == path)
		return NULL;
	ext++; // quita el punto y deja solo la extension

	cmd = get_run_command(ext);
	if (cmd == NULL)
		return NULL;

	sprintf(command, "%s %s", cmd, path);

	for (int i = 0; i < n_args; i++)
		sprintf(command, "%s %s", command, args[i][1]); // el valor de cada argumento

	f = guardar_output(command, length, body, len_body);

	return f;
}

FILE *guardar_output(char exec[1024], long *length, char body[8192], int len_body)
{
	// char buffer[4096];
	// size_t bytes_leidos;
	char *salida = "./outputs/salida.out";

	sprintf(exec, "%s > %s", exec, salida);
	FILE *e = popen(exec, "w");
	fprintf(e, "%.*s", len_body, body);

	pclose(e);

	struct stat st;
	stat(salida, &st);
	*length = st.st_size;

	return fopen(salida, "rb");

	// while ((bytes_leidos = fread(buffer, 1, sizeof(buffer) * sizeof(char), e)) > 0)
	// {
	// 	*length = *length + bytes_leidos;
	// 	fwrite(buffer, sizeof(char), bytes_leidos, f);
	// }

	// fclose(f);
}

char *get_run_command(char *ext)
{
	if (strcmp(ext, "py") == 0)
		return "python3";
	else if (strcmp(ext, "php") == 0)
		return "php";

	return NULL;
}