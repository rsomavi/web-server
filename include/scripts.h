#ifndef SCRIPTS_H
#define SCRIPTS_H

#include <stdio.h>
#include "http_parser.h"

/**
 * @brief Executes a script file and returns a file pointer to its output.
 *
 * This function determines the appropriate interpreter based on the file extension,
 * constructs the command with any provided arguments, executes the script,
 * and returns a file pointer to read the output.
 *
 * @param path The path to the script file to execute.
 * @param length Pointer to store the length of the script output.
 * @param args Array of argument key-value pairs to pass to the script.
 * @param n_args The number of arguments in the args array.
 * @param body The request body content used as stdin for the script.
 * @param len_body The length of the body content.
 *
 * @return FILE pointer to the script output stream, or NULL on error.
 */
FILE *run_script(char path[256], long *length, char args[MAX_ARGUMENTOS][2][50], int n_args, char body[8192], int len_body);

#endif