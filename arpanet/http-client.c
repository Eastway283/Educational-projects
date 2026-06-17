#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <asm-generic/socket.h>
#include <netdb.h>

#define PORT 80
#define PROTOCOL "http://"

/* parse_url - извлекает имя хоста и путь из URL 
 * возвращает ненулевое значение в случае ошибки */
int parse_url(const char *url, char *host, char *path);

/* connect_to_host - подключается к переданному порту хоста
 * возвращает дескриптор сокета, -1 в случае ошибки */
int connect_to_host(const char *host, int port);

/* send_request - формирует и отправляет http запрос по указанному сокету
 * host - имя хоста, path - путь к файлу. -1 в случае ошибки */
int send_request(int sock, const char *host, const char *path);

/* receive_response - получает ответ от сервера по заданному sock
 * выводит ответ в STDOUT_FILENO, игнорируя заголовки
 * автоматически закрывает сокет */
void receive_response(int sock);

int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s URL\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char host[256], path[512];
    if (parse_url(argv[1], host, path) != 0) {
        fprintf(stderr, "%s: invalid URL\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd;
    if ((sockfd = connect_to_host(host, PORT)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if ((send_request(sockfd, host, path)) != 0) {
        perror("send");
        exit(EXIT_FAILURE);
    }

    receive_response(sockfd);


    return 0;
}

int parse_url(const char *url, char *host, char *path) {
    if (!url || !host || !path)
        return 1;
    
    // Хост
    const char *start = strstr(url, PROTOCOL);
    if (!start)
        return 1;
    // Пропускаем протокол
    start += strlen(PROTOCOL);
    const char *end = strchr(start, '/');

    if (!end) { // Только хост
        strcpy(host, start);
        strcpy(path, "/");
        return 0;
    }
    size_t len = end - start;
    strncpy(host, start, len);
    host[len] = '\0';

    // Путь
    start = end;
    strcpy(path, start);
    return 0;
}

int connect_to_host(const char *host, int port) {
    if (!host || port <= 0)
        return -1;

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // IPv4 или IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP 

    char strport[6];
    snprintf(strport, sizeof(strport), "%d", port);

    /* Получение связного списка адресов, соответствующих хосту и порту */
    int err = getaddrinfo(host, strport, &hints, &res);
    if (err) {
        gai_strerror(err); // возвращает текстовое описание ошибки DNS/адреса
        return -1;
    }

    int sockfd = -1;
    struct addrinfo *p;
    /* Перебираем все адреса, пока не удастся подключиться */
    for (p = res; p; p = p->ai_next) {
        /* Создаем сокет с параметрами текущего адреса */
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0)
            continue; // Не удалось создать, пробуем следующий адрес

        /* Пытаемся установить TCP-соединение */
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == 0)
            break;

        /* Не удалось, закрываем сокет и пробуем дальше */
        close(sockfd);
        sockfd = -1;
    }
    /* Освобождаем память, выделенную getaddrinfo */
    freeaddrinfo(res);
    return sockfd;
}

int send_request(int sock, const char *host, const char *path) {
    if (sock < 0 || !host || !path)
        return -1;

    char request[512];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n"
             "\r\n",
             path, host);
    if (send(sock, request, strlen(request), 0) == -1)
        return -1;
    return 0;
}

void receive_response(int sock) {
    if (sock < 0)
        return;
    char buff[BUFSIZ];
    ssize_t n;
    int header_skipped = 0;

    while ((n = recv(sock, buff, BUFSIZ - 1, 0)) > 0) {
        char *body_start = buff;
        ssize_t body_len = n;
        if (!header_skipped) {
            buff[n] = '\0';
            char *delim = strstr(buff, "\r\n\r\n");
            if (delim) {
                body_start = delim + 4;
                body_len = n - (body_start - buff);
                header_skipped = 1;
            } else 
                continue;
        }
        if (body_len > 0)
            write(STDOUT_FILENO, body_start, body_len);
    }
    if (n < 0)
        perror("recv");
    close(sock);
}
