int epoll_socket_create(char *host, int port) {
    int socket_fd;
    struct sockaddr_in server_addr;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        LOG_ERROR("socket error");
        return -1;
    }

    int opt = IP_PMTUDISC_WANT;
    if (setsockopt(socket_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        LOG_ERROR("setsockopt error");
        return -1;
    }


    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(host);
    server_addr.sin_port = htons(port);

    if (bind(socket_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("bind error");
        return -1;
    }

    if (listen(socket_fd, CONN_REQ_QUEUE) == -1) {
        perror("listen error");
        return -1;
    }

    setnonblocking(socket_fd);

    return socket_fd;
}

int epoll_loop(server_t *server) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_socket, epoll_fd, nfds;
    struct epoll_event event, events[MAX_EVENTS];

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        return EXIT_FAILURE;
    }

    setnonblocking(server->socket_fd);

    event.data.fd = server->socket_fd;
    event.events = EPOLLIN;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server->socket_fd, &event) ==
        -1) {
        perror("epoll_ctl: server_socket");
        return EXIT_FAILURE;
    }

    server->is_running = 1;
    while (server->is_running) {
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            close(server->socket_fd);
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == server->socket_fd) {
                client_socket =
                    accept(server->socket_fd,
                           (struct sockaddr *)&client_addr, &client_len);
                if (client_socket == -1) {
                    perror("accept");
                    continue;
                }

                event.data.fd = client_socket;
                event.events = EPOLLIN | EPOLLET;

                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) ==
                    -1) {
                    perror("epoll_ctl: client_socket\n");
                    return EXIT_FAILURE;
                }
            } else {
                conn_data_t *arg = malloc(sizeof(conn_data_t));
                arg->clientfd = events[i].data.fd;
                arg->static_dir = server->static_dir;
                if (threadpool_add(server->tpool, &conn_handler, (void *)arg, 0) < 0) {
                    perror("thpool_add_work err\n");
                    return EXIT_FAILURE;
                }
            }
        }
    }
}
