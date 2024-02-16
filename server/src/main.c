#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "server.h"

server_t* server = NULL;

void signal_handler(int signum)
{
    LOG_INFO("Received signal %d\n", signum);
    server_destroy(server);
    exit(0);
}

int main() {
    log_set_level(LOG_INFO);

    char host[16] = "0.0.0.0";
    char static_dir[MAX_PATH_LEN] = "./static";
    int port = 8080;
    server = server_create(host, static_dir, port);
    if (server == NULL) {
        LOG_FATAL("create server error: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGKILL, signal_handler);

    if (server_serve(server)) {
        server_destroy(server);
        LOG_FATAL("server serve error: %s", strerror(errno));
        return -1;
    }

    server_destroy(server);

    return 0;
}