#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/epoll.h>

#define MAX_EVENTS 64
#define PORT 8080
#define MAX_CLIENTS 10

void *client_handler(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[1024];
    int read_size;

    while ((read_size = recv(client_socket, buffer, 1024, 0)) > 0) {
        write(client_socket, buffer, strlen(buffer));
        memset(buffer, 0, 1024);
    }

    if (read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if (read_size == -1) {
        perror("recv failed");
    }

    close(client_socket);
    free(arg);
    return 0;
}

int main() {
    int server_socket, client_socket, epoll_fd, nfds;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    struct epoll_event event, events[MAX_EVENTS];
    pthread_t client_threads[MAX_CLIENTS];

    // Создание сокета
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Привязка сокета
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return -1;
    }

    listen(server_socket, 5);

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        return 1;
    }

    event.data.fd = server_socket;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event) == -1) {
        perror("epoll_ctl: server_socket");
        return 1;
    }

    while (1) {
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == server_socket) {
                client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
                if (client_socket == -1) {
                    perror("accept");
                    continue;
                }
                event.data.fd = client_socket;
                event.events = EPOLLIN | EPOLLET;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) == -1) {
                    perror("epoll_ctl: client_socket");
                    return 1;
                }
            } else {
                int *arg = (int *)malloc(sizeof(int));
                *arg = events[i].data.fd;
                pthread_create(&client_threads[i], NULL, client_handler, (void *)arg);
            }
        }
    }

    close(server_socket);
    return 0;
}