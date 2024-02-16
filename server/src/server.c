#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "server.h"
#include "log.h"
#include "thpool.h"
#include "epoll.h"

#define THREAD 4
#define THREAD_QUEUE_SIZE 8000

server_t *server_create(char host[HOST_SIZE], char static_dir[MAX_PATH_LEN], int port) {
    // General.
    server_t *server = malloc(sizeof(struct server));
    server->is_running = false;
    strcpy(server->host, host);
    strcpy(server->static_dir, static_dir);
    server->port = port;

    // Thread pool.
    threadpool_t *pool;
    pool = threadpool_create(THREAD, THREAD_QUEUE_SIZE, 0);
    if (pool == NULL) {
        perror("threadpool_create error");
        free(server);
        return NULL;
    }

    server->tpool = pool;
    return server;
}

int server_serve(server_t *server) {
    server->socket_fd = epoll_socket_create(server->host, server->port);
    if (server->socket_fd < 0) {
        perror("epoll_socket_create");
        return EXIT_FAILURE;
    }

    LOG_INFO("Server started at host:%s, port:%d", server->host, server->port);

    if (epoll_loop(server) != 0) {
        perror("epoll_loop");
        return EXIT_FAILURE;
    }

    return 0;
}

void server_destroy(server_t *server) {
    if (server == NULL) {
        return;
    }
    close(server->socket_fd);
    threadpool_destroy(server->tpool, threadpool_graceful);

    LOG_INFO("Server stopped");

    free(server);
}