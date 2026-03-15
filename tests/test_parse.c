#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "http_parser.h"

#define MAX_ARGUMENTOS 10

/* declara tu función */
// int parse_request(int connfd,
//                   char verbo[10],
//                   char archivo[50],
//                   int *num_args,
//                   char args[MAX_ARGUMENTOS][2][50],
//                   char host[100],
//                   char content_type[100],
//                   int *content_length,
//                   char body[1024]);

void run_test(const char *request)
{
    int sv[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, sv);

    write(sv[0], request, strlen(request));

    char verbo[10];
    char archivo[50];
    char host[100];
    char content_type[100];
    char body[1024];
    int content_length;
    int n_args;
    char args[MAX_ARGUMENTOS][2][50];

    printf("\n=================================\n");
    printf("REQUEST:\n%s\n", request);

    if (parse_request(sv[1], verbo, archivo, &n_args, args,
                      host, content_type, &content_length, body) == -1)
    {
        printf("PARSE ERROR\n");
    }
    else
    {
        printf("VERBO: %s\n", verbo);
        printf("ARCHIVO: %s\n", archivo);
        printf("HOST: %s\n", host);
        printf("CONTENT_TYPE: %s\n", content_type);
        printf("CONTENT_LENGTH: %d\n", content_length);
        printf("BODY: %s\n", body);
    }

    close(sv[0]);
    close(sv[1]);
}

int main()
{
    const char *get_request =
        "GET /index.html HTTP/1.1\n"
        "Host: localhost\n"
        "\n";

    const char *post_request =
        "POST /script HTTP/1.1\n"
        "Host: localhost\n"
        "Content-Type: text/plain\n"
        "Content-Length: 4\n"
        "\n"
        "hola";

    const char *put_request =
        "PUT /file.txt HTTP/1.1\n"
        "Host: localhost\n"
        "Content-Type: text/plain\n"
        "Content-Length: 11\n"
        "\n"
        "hola mundo";

    run_test(get_request);
    run_test(post_request);
    run_test(put_request);

    return 0;
}