#define _GNU_SOURCE

#include "http.h"

#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "errno.h"
#include "log.h"

#define HEADER_MAX_LEN 1024
#define RESPONSE_BATCH_SIZE 512

char* http_status_responses[] = {"HTTP/1.1 200 OK",
                                 "HTTP/1.1 400 Bad Request\r\n\r\n",
                                 "HTTP/1.1 403 Forbidden\r\n\r\n",
                                 "HTTP/1.1 404 Not Found\r\n\r\n",
                                 "HTTP/1.1 405 Method Not Allowed\r\n\r\n",
                                 "HTTP/1.1 500 Internal Server Error\r\n\r\n"};

char* TYPE_EXT[MIME_TYPES_COUNT] = {"txt", "css",  "html", "js", "png",
                                    "jpg", "jpeg", "swf",  "gif"};
char* MIME_TYPE[MIME_TYPES_COUNT] = {
    "text/plain", "text/css",   "text/html",  "text/javascript",
    "image/png",  "image/jpeg", "image/jpeg", "application/x-shockwave-flash",
    "image/gif"};

int parse_req(http_request_t* req, char* buff) {
    // Находим разделительы между методом и URL
    char* method_end = strstr(buff, " ");
    char* url_start = method_end + 1;
    char* url_end = strstr(url_start, " ");

    // Получаем длины строк метода и URL
    int method_len = method_end - buff;
    int url_len = url_end - url_start;

    // Извлекаем метод из буфера и преобразуем его в перечисление
    // request_method_t
    char method_str[method_len];
    strncpy(method_str, buff, method_len);
    method_str[method_len] = '\0';

    if (strcmp(method_str, "GET") == 0) {
        req->method = GET;
    } else if (strcmp(method_str, "HEAD") == 0) {
        req->method = HEAD;
    } else {
        req->method = UNSUPPORTED_METHOD;
    }

    // Копируем URL из буфера в поле структуры
    strncpy(req->url, url_start, url_len);
    req->url[url_len] = '\0';

    return 0;
}

int write_response(int fd, const void* buf, size_t n) {
    int byte_write = write(fd, buf, n);
    if (byte_write < 0) {
        LOG_ERROR("write_response: %s", strerror(errno));
    }
    return byte_write;
}

char* get_type(char* path) {
    char* res = path + strlen(path) - 1;
    while (res >= path && *res != '.' && *res != '/') {
        res--;
    }

    if (res < path || *res == '/') {
        return NULL;
    }
    return ++res;
}

char* get_content_type(char* path) {
    char* ext = get_type(path);
    if (ext == NULL) {
        return NULL;
    }

    int i = 0;
    for (i = 0; i < MIME_TYPES_COUNT && strcmp(TYPE_EXT[i], ext) != 0; i++)
        ;
    if (i >= MIME_TYPES_COUNT) return "application/octet-stream";

    return MIME_TYPE[i];
}

int send_headers(char* path, int clientfd) {
    char *status = http_status_responses[STATUS_SUCCESS],
         connection[] = "Connection: close";
    char *len = calloc(HEADER_MAX_LEN, sizeof(char)),
         *type = calloc(HEADER_MAX_LEN, sizeof(char));
    char* res_str = "/0";

    if (len == NULL || type == NULL) {
        LOG_ERROR("failed to alloc headers buffs");
        return -1;
    }

    struct stat file_stat;
    if (stat(path, &file_stat) < 0) {
        LOG_ERROR("stat error: %s", strerror(errno));
        return -1;
    }
    sprintf(len, "Content-Length: %ld", file_stat.st_size);
    char* mime_type = get_content_type(path);

    int rc = 0;
    if (mime_type == NULL) {
        LOG_WARN("could not determine the file type");
        rc = asprintf(&res_str, "%s\r\n%s\r\n%s\r\n\r\n", status, connection,
                      len);
    } else {
        sprintf(type, "Content-Type: %s", mime_type);
        rc = asprintf(&res_str, "%s\r\n%s\r\n%s\r\n%s\r\n\r\n", status,
                      connection, len, type);
    }
    if (rc < 0) {
        LOG_ERROR("formation of headers of http response failed");
        return -1;
    }

    int byte_write = write_response(clientfd, res_str, rc);
    free(res_str);
    if (byte_write < 0) {
        return -1;
    }

    return 0;
}

void send_status(int clientfd, int status) {
    if (clientfd == -1) {
        LOG_ERROR("send_status: bad clientfd %d", clientfd);
        close(clientfd);
        return;
    }

    char* response = http_status_responses[status];
    write_response(clientfd, response, strlen(response));
}

void send_file(char* path, int clientfd) {
    int fd = open(path, 0);
    if (fd < 0) {
        LOG_ERROR("open: %s", strerror(errno));
        return;
    }

    char* buff_resp = calloc(RESPONSE_BATCH_SIZE, sizeof(char));
    if (buff_resp == NULL) {
        LOG_ERROR("failed alloc resp buf: %s", strerror(errno));
        return;
    }
    unsigned long long total_read = 0, total_write = 0;
    long byte_write = 0, byte_read = 0;

    while (1) {
        byte_read = read(fd, buff_resp, RESPONSE_BATCH_SIZE);
        if (byte_read < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                LOG_WARN("read: %s; errno = %d", strerror(errno), errno);
                usleep(10000);  // 10ms
                continue;
            }
            LOG_ERROR("read: %s; errno = %d", strerror(errno), errno);
            break;
        } else if (byte_read == 0) {
            break;  // end of file
        }
        total_read += byte_read;

        ssize_t total_write_temp = 0;
        while (total_write_temp < byte_read) {
            byte_write = write(clientfd, buff_resp + total_write_temp,
                               byte_read - total_write_temp);
            if (byte_write < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    LOG_WARN("write: %s; errno = %d", strerror(errno), errno);
                    usleep(10000);  // 10ms
                    continue;
                }
                LOG_ERROR("write: %s; errno = %d", strerror(errno), errno);
                break;
            }
            total_write_temp += byte_write;
        }
        if (total_write_temp != byte_read) {
            break;
        } else {
            total_write += total_write_temp;
        }
    }

    if (total_read == total_write) {
        LOG_INFO("Response successed");
    } else {
        LOG_ERROR("failed to write entire file: read = %d; write = %d",
                  total_read, total_write);
    }

    close(fd);
}

void process_get_request(char* path, int clientfd) {
    if (send_headers(path, clientfd) < 0) return;

    send_file(path, clientfd);
}

void process_head_request(char* path, int clientfd) {
    send_headers(path, clientfd);
}

void send_response(char* path, int clientfd, http_method_t method) {
    switch (method) {
        case GET:
            process_get_request(path, clientfd);
            break;
        case HEAD:
            process_head_request(path, clientfd);
            break;
        default:
            LOG_ERROR("unsupported http method");
            send_status(clientfd, STATUS_ERROR_NOT_ALLOWED);
            break;
    }
}

void handle_http_request(int clientfd, http_request_t* req, char* static_dir) {
    char* path = calloc(MAX_URL_LEN, sizeof(char));
    if (path == NULL) {
        LOG_ERROR("failder alloc path buff: %s", strerror(errno));
        return;
    }

LOG_INFO("req url: %s", req->url);
    const char* rel_path = req->url[0] == '/' ? req->url + 1 : req->url;
    if (strlen(rel_path) == 0) {
        rel_path = "index.html";
    }
    // LOG_INFO("rel_path: %s", rel_path);

    char* full_path = calloc(MAX_PATH_LEN, sizeof(char));
    if(full_path == NULL) {
        send_status(clientfd, STATUS_ERROR_INTERNAL_SERVER_ERROR);
        LOG_ERROR("failed to allocate memory for full path: %s", strerror(errno));
        free(path);
        return;
    }

    snprintf(full_path, MAX_PATH_LEN, "%s/%s", static_dir, rel_path);
    // LOG_INFO("full_path: %s", full_path);

    if (realpath(full_path, path) == NULL) {
        if (errno == ENOENT) {
            send_status(clientfd, STATUS_ERROR_NOT_FOUND);
        } else {
            send_status(clientfd, STATUS_ERROR_INTERNAL_SERVER_ERROR);
        }
        LOG_ERROR("realpath %s: %s", path, strerror(errno));
        free(path);
        return;
    }
    // LOG_INFO("realpath: %s", path);

    char* trimmed_dir;
    if (strncmp(static_dir, "./", 2) == 0) {
        trimmed_dir = static_dir + 2;
    } else {
        trimmed_dir = static_dir;
    }

    if (strstr(path, trimmed_dir) == NULL) {
        send_status(clientfd, STATUS_ERROR_FORBIDDEN);
        LOG_ERROR("attempt to access outside the root");
        free(path);
        return;
    }

    send_response(path, clientfd, req->method);
}