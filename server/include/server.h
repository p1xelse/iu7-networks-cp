#include "thpool.h"
#include "log.h"

#define HOST_SIZE 16
#define MAX_PATH_LEN 256

struct server {
    char host[HOST_SIZE];
    int port;
    int is_running;
    int socket_fd;
    char static_dir[MAX_PATH_LEN];

    threadpool_t *tpool;
};

typedef struct server server_t;

void handle_request(void *arg);
server_t *server_create(char host[HOST_SIZE], char static_dir[MAX_PATH_LEN], int port);
int server_serve(server_t *server);
void server_destroy(server_t *server);