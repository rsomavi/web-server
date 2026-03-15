#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "http_parser.h"

int main()
{
char archivo[50];
int num_args;
char args[MAX_ARGUMENTOS][2][50];

/* ===============================
   TEST 1: sin argumentos
   =============================== */
strcpy(archivo, "index.html");

get_url_args(archivo, &num_args, args);

assert(strcmp(archivo, "index.html") == 0);
assert(num_args == 0);

printf("[OK] TEST 1 sin argumentos\n");

/* ===============================
   TEST 2: un argumento
   =============================== */
strcpy(archivo, "page.html?user=ana");

get_url_args(archivo, &num_args, args);

assert(strcmp(archivo, "page.html") == 0);
assert(num_args == 1);
assert(strcmp(args[0][0], "user") == 0);
assert(strcmp(args[0][1], "ana") == 0);

printf("[OK] TEST 2 un argumento\n");

/* ===============================
   TEST 3: múltiples argumentos
   =============================== */
strcpy(archivo, "test.html?a=1&b=2&c=hola");

get_url_args(archivo, &num_args, args);

assert(strcmp(archivo, "test.html") == 0);
assert(num_args == 3);

assert(strcmp(args[0][0], "a") == 0);
assert(strcmp(args[0][1], "1") == 0);

assert(strcmp(args[1][0], "b") == 0);
assert(strcmp(args[1][1], "2") == 0);

assert(strcmp(args[2][0], "c") == 0);
assert(strcmp(args[2][1], "hola") == 0);

printf("[OK] TEST 3 múltiples argumentos\n");

/* ===============================
   TEST 4: argumento mal formado (=valor)
   =============================== */
strcpy(archivo, "bad.html?=oops&x=1");

get_url_args(archivo, &num_args, args);

assert(strcmp(archivo, "bad.html") == 0);
assert(num_args == 1);
assert(strcmp(args[0][0], "x") == 0);
assert(strcmp(args[0][1], "1") == 0);

printf("[OK] TEST 4 ignora mal formados\n");

/* ===============================
   TEST 5: argumento mal formado (clave=)
   =============================== */
strcpy(archivo, "bad2.html?clave=&ok=si");

get_url_args(archivo, &num_args, args);

assert(num_args == 1);
assert(strcmp(args[0][0], "ok") == 0);
assert(strcmp(args[0][1], "si") == 0);

printf("[OK] TEST 5 ignora valor vacío\n");

printf("\n Todos los tests de get_url_args pasaron\n");
return 0;

}
