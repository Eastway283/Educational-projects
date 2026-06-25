#include "server.h"
#include <linux/limits.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <stdlib.h>

/* массив пар (ext, type) для определения mime-типа */
static mime_type mime_types[] = {
    ".html",  "text/html",
    ".css" ,  "text/css",
    ".js"  ,  "application/javascript",
    ".png" ,  "image/png",
    ".jpg" ,  "image/jpeg",
    ".jpeg",  "image/jpeg",
    ".gif" ,  "image/gif",
    ".svg" ,  "image/svg+xml",
    ".txt" ,  "text/plain",
    ".pdf" ,  "application/pdf"
};

static const size_t mime_typessize = 10;

/* неизвестное расширение */
static const char *unknown_ext = "application/octet-stream";

/* массив пар (code, text) для определения ошибки */
static err_type err_types[] = {
    200, "OK",
    400, "Bad Request",
    403, "Forbidden",
    404, "Not Found",
    405, "Method Not Allowed",
    500, "Internal Server Error"
};

static const size_t err_typessize = 6;

#define othererr err_types[5]

/* статитческие буферы для проверки корня и распложения файла */
static char fullroot[PATH_MAX] = {0};
static char fullpath[PATH_MAX] = {0};

/* статический буфер для отправки заголовка и файла */
static char header[256];
static char body[256];
static char fbuf[BUFSIZ];

/* статический буфер для списка файлов в каталоге */
#define LSIZE (BUFSIZ * 2)
static char list[LSIZE] = {0};
static size_t listlen = 0;
static size_t remaining = LSIZE;

const char *status_text(int code) {
    for (size_t i = 0; i < err_typessize; i++)
        if (code == err_types[i].code)
            return err_types[i].text;
    return othererr.text;
}

int create_server_sock(int port) {

    if (port < 1 || port > MAXPORT)
        return -1;

    /* создаем TCP сокет с IPv4 */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        return -1;
    /* устанавливаем параметры сокета и разрешаем переиспользовать порт */
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(sockfd);
        return -1;
    }
    /* задаем параметры соединения */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;         // IPv4
    addr.sin_addr.s_addr = INADDR_ANY; // слушаем на всех интерфейсах
    addr.sin_port = htons(port);       // преобразуем порт в сетевой порядок

    /* привязываем сокет к адресу */
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sockfd);
        return -1;
    }
    /* начинаем слушать (USER_COUNT - размер очереди ожидающих) */
    if (listen(sockfd, USER_COUNT) < 0) {
        close(sockfd);
        return -1;
    }

    return sockfd;
}

const char *get_mime_type(const char *path) {
    if (!path)
        return NULL;
    /* ищем начало расширения файла */
    const char *ext = strrchr(path, '.');
    /* расширение не найдено */
    if (!ext)
        return unknown_ext;

    /* проходим по массиву и ищем совпадение */
    for (size_t i = 0; i < mime_typessize; i++)
        if (strcmp(ext, mime_types[i].ext) == 0) /* найдено */
            return mime_types[i].type;

    return unknown_ext;
}

int check_path(const char *path, const char *root) {
    if (!path || !root)
        return 0;
    /* такого пути не существует */
    if (!realpath(root, fullroot))
        return 0;
    if (!realpath(path, fullpath))
        return 0;
    /* проверяем, находится ли файл в данном корне */
    size_t rootlen = strlen(fullroot);
    if (strncmp(fullroot, fullpath, rootlen) != 0)
        return 0;
    /* проверка на запрос корня или подкаталога */
    if (fullpath[rootlen] != '\0' && fullpath[rootlen] != '/')
        return 0;
    /* проверка возможности доступа к файлу */
    if (access(fullpath, R_OK) != 0)
        return 0;
    struct stat buf;
    if (stat(fullpath, &buf) != 0)
        return 0;
    return 1;
}

int send_error(int client_fd, int status_code)
{
    const char *err_text = status_text(status_code);
    ssize_t body_len = snprintf(body, sizeof(body),
            "<html><head><meta charset=\"UTF-8\"><title>%s</title></head>"
            "<body><h1>%s</h1></body></html>", err_text, err_text);
    /* формируем заголовок */
    ssize_t headerlen;
    headerlen = snprintf(header, sizeof(header),
            "HTTP/1.1 %d %s\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n"
            "\r\n\r\n",
            status_code, err_text, body_len);
    /* отправляем клиенту */
    if (write(client_fd, header, headerlen) != headerlen)
        return 0;
    if (write(client_fd, body, body_len) != body_len)
        return 0;
    return 1;
}

int send_file(int client_fd, const char *path) {
    if (!path)
        return 0;
    /* получаем MIME-тип файла */
    const char *type = get_mime_type(path);
    /* открываем файл и получаем его размер в байтах */
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return 0;
    struct stat finfo;
    if (fstat(fd, &finfo) != 0) {
        close(fd);
        return 0;
    }
    /* формируем и отправляем HTTP-заголовок */
    ssize_t headerlen = snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK \r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %ld\r\n"
            "Connection: close\r\n"
            "\r\n",
            type, (long)finfo.st_size);
    if (write(client_fd, header, headerlen) != headerlen) {
        close(fd);
        return 0;
    }
    /* читаем и отправляем файла клиенту */
    size_t n;
    while ((n = read(fd, fbuf, BUFSIZ)) > 0)
        if (write(client_fd, fbuf, n) != n) {
            close(fd);
            return 0;
        }
    /* закрываем дескриптор файла */
    close(fd);
    return 1;
}

/* dirwalk - вспомогательная функция для формирования листинга
 * открывает и читает каталог, применяя к каждому элементу fcn */
static void dirwalk(const char *dir, void (*fcn) (const char *)) {
    char name[PATH_MAX];
    struct dirent *dp;
    DIR *dfd;

    /* открываем и получаем информацию о каталоге */
    if ((dfd = opendir(dir)) == NULL) {
        fprintf(stderr, "dirwalk: can't open %s\n", dir);
        return;
    }
    /* читаем каждую запись каталога и вызываем fcn */
    while ((dp = readdir(dfd)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue; /* пропускаем себя и родителя */
        if (strlen(dp->d_name) + strlen(dir) + 2 > PATH_MAX) {
            fprintf(stderr, "dirwalk: name %s/%s too long\n", dir, dp->d_name);
            continue;
        } else { /* формируем относительное имя файла */
            sprintf(name, "%s/%s", dir, dp->d_name);
            (*fcn)(name);
        }
    }
    closedir(dfd);
}

/* finfo - вспомогательная функция для формирования листинга. Формирует тело http листинга
 * увеличивая listen */
static void finfo(const char *name) {
    struct stat st;
    /* проверяем возможность доступа к файлу */
    if (stat(name, &st) == - 1) {
        fprintf(stderr, "finfo: can't access %s\n", name);
        return;
    }
    /* обрезаем имя файла до относительного */
    const char *shrtname = strrchr(name, '/');
    const char *output_name = (shrtname) ? shrtname + 1 : name;  // +1 чтобы пропустить '/'

    char display_name[PATH_MAX];
    if (S_ISDIR(st.st_mode)) {
        /* для папок добавляем '/' в конце */
        snprintf(display_name, sizeof(display_name), "%s/", output_name);
    } else {
        snprintf(display_name, sizeof(display_name), "%s", output_name);
    }

    /* получаем время последней модификации и удаляем \n */
    char timebuf[128];
    struct tm *tm_info = localtime(&st.st_mtime);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M", tm_info);

    /* размер в килобайтах (целочисленно) */
    long size_kb = st.st_size / 1024;

    /* формируем html строку с ссылкой и увеличиваем текущий размер буфера */
    listlen += snprintf(list + listlen, remaining,
            "<tr><td><a href=\"%s\">%s</a></td><td>%ld KB</td><td>%s</td></tr>",
            display_name, display_name, size_kb, timebuf);
    /* уменьшаем значение свободного места */
    remaining = LSIZE - listlen;
}

int send_dir(int client_fd, const char *path) {
    /* устанавливаем размеры буфера листинга */
    listlen = 0, remaining = LSIZE;
    listlen += snprintf(list, remaining,
            "<!DOCTYPE html>\n<html><head><meta charset=\"UTF-8\">"
            "<title>Index of %s</title></head>\n<body>\n<h1>Index of %s</h1>\n"
            "<hr>\n<table cellpadding=\"5\">\n<tr><th>Name</th><th>Size</th><th>Modified</th></tr>\n",
            path, path);
    remaining = LSIZE - listlen;
    listlen += snprintf(list + listlen, remaining,
            "<tr><td><a href=\"../\">Parent Directory</a></td><td>-</td><td>-</td></tr>\n");
    remaining = LSIZE - listlen;
    /* проходим по содержимому каталога и добавляем в листинг */
    dirwalk(path, finfo);
    listlen += snprintf(list + listlen, remaining,
            "</table>\n<hr>\n</body>\n</html>\n");
    /* формируем заголовок и отправляем клиенту */
    ssize_t headerlen = snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n\r\n", listlen);
    if (write(client_fd, header, headerlen) != headerlen)
        return 0;
    if (write(client_fd, list, listlen) != listlen)
        return 0;
    return 1;
}
