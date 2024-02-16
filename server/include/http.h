#ifndef SERVER_HTTP_H
#define SERVER_HTTP_H

#define MAX_URL_LEN 256
#define MAX_PATH_LEN 256

#define STATUS_SUCCESS 0
#define STATUS_ERROR_BAD_REQUEST 1
#define STATUS_ERROR_FORBIDDEN 2
#define STATUS_ERROR_NOT_FOUND 3
#define STATUS_ERROR_NOT_ALLOWED 4
#define STATUS_ERROR_INTERNAL_SERVER_ERROR 5

#define MIME_TYPES_COUNT 9

extern char* TYPE_EXT[MIME_TYPES_COUNT];
extern char* MIME_TYPE[MIME_TYPES_COUNT];

typedef enum http_method { GET, HEAD, UNSUPPORTED_METHOD } http_method_t;

typedef struct http_request {
    http_method_t method;
    char url[MAX_URL_LEN];
} http_request_t;

int parse_req(http_request_t* req, char* buff);
void handle_http_request(int clientfd, http_request_t* req, char* static_dir);
void send_status(int clientfd, int status);

#endif  // SERVER_HTTP_H