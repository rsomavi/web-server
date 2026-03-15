#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <unistd.h>

#include "http_parser.h"

#define MAX_ARGUMENTOS 10

/* Helper: envía una request por un socket */
void send_request(int fd, const char *req)
{
   write(fd, req, strlen(req));
}

int main() {
   int sv[2];
   char verbo[10];
   char archivo[50];
   int num_args;
   char args[MAX_ARGUMENTOS][2][50];

   /* Crear socket par (simula cliente-servidor) */
   assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0);

   /* ===============================
      TEST 1: GET simple
      =============================== */
   const char *req1 = "GET /index.html HTTP/1.1\r\n\r\n";
   send_request(sv[0], req1);

   assert(parse_request(sv[1], verbo, archivo, &num_args, args) == 0);
   assert(strcmp(verbo, "GET") == 0);
   assert(strcmp(archivo, "index.html") == 0);
   assert(num_args == 0);

   printf("[OK] TEST 1 GET simple\n");

   /* ===============================
      TEST 2: GET con argumentos
      =============================== */
   const char *req2 =
      "GET /page.html?user=ana&id=3 HTTP/1.1\r\n\r\n";

   send_request(sv[0], req2);

   assert(parse_request(sv[1], verbo, archivo, &num_args, args) == 0);
   assert(strcmp(verbo, "GET") == 0);
   assert(strcmp(archivo, "page.html") == 0);
   assert(num_args == 2);
   assert(strcmp(args[0][0], "user") == 0);
   assert(strcmp(args[0][1], "ana") == 0);
   assert(strcmp(args[1][0], "id") == 0);
   assert(strcmp(args[1][1], "3") == 0);

   printf("[OK] TEST 2 GET con argumentos\n");

   /* ===============================
      TEST 3: verbo no soportado
      =============================== */
   const char *req3 =
      "PUT /index.html HTTP/1.1\r\n\r\n";

   send_request(sv[0], req3);

   assert(parse_request(sv[1], verbo, archivo, &num_args, args) == -1);

   printf("[OK] TEST 3 verbo no soportado\n");

   /* ===============================
      TEST 4: versión HTTP incorrecta
      =============================== */
   const char *req4 =
      "GET /index.html HTTP/2.0\r\n\r\n";

   send_request(sv[0], req4);

   assert(parse_request(sv[1], verbo, archivo, &num_args, args) == -1);

   printf("[OK] TEST 4 version incorrecta\n");

   close(sv[0]);
   close(sv[1]);

   printf("\n Todos los tests de parse_request pasaron\n");
   return 0;
}
