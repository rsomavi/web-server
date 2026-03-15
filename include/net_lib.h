#ifndef NET_LIB_H
#define NET_LIB_H

#include <sys/socket.h>
#include <netinet/in.h>

/** 
 * @brief Initializes the server socket, binds it to a port, and starts listening.
 * 
 * This function creates a TCP socket (SOCK_STREAM) using IPv4 (AF_INET).
 * It associates the socket with any available interface (INADDR_ANY) and the 
 * specified port, then sets the maximum queue for pending connections.
 * 
 * @param puerto The port number where the server will listen for connections.
 * @param backlog Maximum length of the queue of pending requests.
 * @return int The file descriptor of the listening socket, or -1 on error.
 */
int open_server(int port, int backlog);

/**
 * @brief Waits for and accepts an incoming client connection.
 * 
 * This is a blocking function that remains idle until a client initiates a connection.
 * It creates a new socket specifically for the communication with the connected client,
 * leaving the original listener socket free to continue accepting other requests.
 * 
 * @param listen_fd The file descriptor of the server's listening socket.
 * @return int A new file descriptor for the client connection, or -1 on error.
 */
int accept_connection(int listen_fd);

#endif