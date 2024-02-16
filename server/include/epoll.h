#ifndef SERVER_EPOLL_H
#define SERVER_EPOLL_H

int epoll_socket_create(char *host, int port);
int epoll_loop(server_t *server);

#endif // SERVER_EPOLL_H