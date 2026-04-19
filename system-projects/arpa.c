#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#define BUFFER_SIZE 16384 // достаточно для HTML-страницы wttr.in

// Извлекает текст с температурой
void extract_temp(const char *html, char *output, size_t out_size);

int main(int argc, char **argv) {
    // 1. Указываем к кому подключаемся
    const char *city = (argc > 1) ? argv[1] : "Minsk";
    const char *host = "wttr.in";
    const char *port = "80";
    char path[256];

    // Формируем путь
    sprintf(path, "/%s?format=%%t&T", city);

    // 2. getaddrinfo - переводим dns в ip и заполняем структуру sockaddr
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // разрешаем IPv4, IPv6
    hints.ai_socktype = SOCK_STREAM; // потоковый сокет - TCP

    int err;
    if ((err = getaddrinfo(host, port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
        exit(1);
    }

    // 3. socket() - создаем дескриптор сокета
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        perror("socket");
        freeaddrinfo(res);
        exit(1);
    }

    // 4. connect() - устанавливаем соединение с удаленной машиной
    // Структура res->ai_addr содержит IP и порт (уже в сетевом порядке байт)
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
        perror("connect");
        close(sockfd);
        freeaddrinfo(res);
        exit(1);
    }

    freeaddrinfo(res); // адресная информация больше не нужна

    // 5. Формируем HTTP - запрос (это просто текст)
    char request[512];
    snprintf(request, sizeof(request),
         "GET %s HTTP/1.0\r\n"
         "Host: %s\r\n"
         "User-Agent: Mozilla/5.0\r\n"
         "Accept: text/html\r\n"
         "\r\n",
         path, host);

    // 6. send() - отправляем запрос (аналогично write)
    if (send(sockfd, request, strlen(request), 0) == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    }

    // 7. recv() - читаем ответб пока соединение не закроют (аналог read)
    char full_response[BUFFER_SIZE];
    size_t total = 0;
    ssize_t n;
    char chunk[BUFFER_SIZE];

  while ((n = recv(sockfd, chunk, sizeof(chunk) - 1, 0)) > 0) {
        chunk[n] = '\0';
        size_t remaining = BUFFER_SIZE - total - 1;
        if ((size_t)n > remaining) n = remaining;
        memcpy(full_response + total, chunk, n);
        total += n;
        full_response[total] = '\0';
        if (remaining <= 0) break;
    }

    if (n == -1) {
        perror("recv");
        close(sockfd);
        return 1;
    }
    // 8. закрываем sockfd как обычный файл
    close(sockfd);

    // 9. Извлекаем температуру
    char temp[128];
    extract_temp(full_response, temp, sizeof(temp));
    printf("%s\n", temp);

    return 0;
}

void extract_temp(const char *html, char *output, size_t out_size)
{
    const char *start = strstr(html, "<div class=\"term-container\">");
    if (!start) {
        sprintf(output, "Not found");
        return;
    }

    start += strlen("<div class=\"term-container\">");
    const char *end = strstr(start, "</div>");
    if (!end) {
        sprintf(output, "Not found");
        return;
    }

    size_t len = end - start;
    if (len >= out_size)
        len = out_size - 1;
    strncpy(output, start, len);
    output[len] = '\0';

    // Удаляем начальные и конечные пробелы/переводы строк
    char *p = output;
    while (*p == ' ' || *p == '\n' || *p == '\r')
        p++;
    memmove(output, p, strlen(p) + 1);
    char *q = output + strlen(output) - 1;
    while (q > output && (*q == ' ' || *q == '\n' || *q == '\r'))
        q--;
    *(q + 1) = '\0';
}
