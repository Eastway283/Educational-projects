#include "url.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>

int parse_url(url_info *urlinfo, const char *url) {
    if (!urlinfo || !url)
        return 1;

    /* начинаем с протокола */
    const char *schm_end = strstr(url, "://");
    if (!schm_end) { // если не найдено, устанавливаем http и port
        memcpy(urlinfo->scheme, DEFAULT_SCHEME, sizeof(DEFAULT_SCHEME));
        urlinfo->scheme[sizeof(urlinfo->scheme) - 1] = '\0';
        urlinfo->port = HTTP_DPORT;
    }
    else { // проверяем протокол и устанавливаем его
        size_t len = schm_end - url;
        strncpy(urlinfo->scheme, url, len);
        urlinfo->scheme[len] = '\0';
        url = schm_end + 3;
        if (strncmp(urlinfo->scheme, "http", len) == 0)
            urlinfo->port = HTTP_DPORT;
        else
            urlinfo->port = HTTPS_DPORT;
    }

    /* Хост */
    if (strstr(url, "user:") != NULL) // пропускаем userinfo
        url += strlen("user:");       // если есть

    /* ищем конец хоста */
    const char *host_end = strpbrk(url, "/:");
    if (!host_end) { // пути нет, устанавливаем DEFAULT_PATH
        strcpy(urlinfo->host, url);
        strcpy(urlinfo->path, DEFAULT_PATH);
        return 0;
    } else { // путь есть, достаем host
        size_t len = host_end - url;
        if (!len)
            return 1;
        strncpy(urlinfo->host, url, len);
        urlinfo->host[len] = '\0';
    }

    /* Проверка на наличие порта */
    if (*host_end == ':') { // Порт есть
        const char *port_start = host_end + 1;
        const char *port_end = strpbrk(port_start, "/?#");
        if (!port_end) { // Дальше ничего нету, стандартный путь
            urlinfo->port = atoi(port_start);
            strcpy(urlinfo->path, DEFAULT_PATH);
            return 0;
        } else { // преобразуем порт из строки в число
            char portstr[6];
            size_t plen = port_end - port_start;
            if (plen >= sizeof(portstr)) // слишком большой порт
                return 1;
            memcpy(portstr, port_start, plen);
            portstr[plen] = '\0';
            urlinfo->port = atoi(portstr);
            if (*(port_end + 1)) // есть путь, копируем все до конца
                strcpy(urlinfo->path, port_end);
            else // стандартный путь
                strcpy(urlinfo->path, DEFAULT_PATH);
        }
    } else { // Порта нету, сразу следует путь
        strcpy(urlinfo->path, host_end);
        return 0;
    }

    /* проверка диапазона порта */
    if (urlinfo->port < 1 || urlinfo->port > MAX_PORT)
        return 1;

    return 0;
}

/* initBuffer - инициализация буффера buffer_t */
void initBuffer(buffer_t *buf) {
    if (buf) {
        buf->count = 0;
        buf->head = buf->tail = buf->slots;
    }
}

/* copyItem - вспомогательная функция копирования url_info */
static void copyItem(url_info *dst, const url_info *src) {
    strcpy(dst->scheme, src->scheme);
    strcpy(dst->host, src->host);
    dst->port = src->port;
    strcpy(dst->path, src->path);
}

/* putBuffer - помещает item в буффер. ненулевое значение в случае ошибки */
int putBuffer(buffer_t *buf, const url_info *item) {
    if (!buf || !item)
        return -1;
    if (buf->count == BUFFSIZE)
        return -1; // места нету
    copyItem(buf->tail++, item);
    /* достигли конца - закольцовываем */
    if (buf->tail == buf->slots + BUFFSIZE)
        buf->tail = buf->slots;
    ++buf->count;
    return 0;
}

/* getBuffer - помещает значение в item из головы buf, ненулевое значение в случае ошибки */
int getBuffer(buffer_t *buf, url_info *item) {
    if (!buf || !item)
        return -1;
    if (!buf->count)
        return -1;
    copyItem(item, buf->head++);
    /* достигли конца - закольцовываем */
    if (buf->head == buf->slots + BUFFSIZE)
        buf->head = buf->slots;
    --buf->count;
    return 0;
}

int connect_to_host(const char *host, int port) {
    if (!host || port < 1 || port > MAX_PORT)
        return -1;

    /* hints - вспомогательная структура для определения типа сокета и версии IP */
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // IPv4 или IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP

    /* переводим порт в строку для getaddrinfo */
    char strport[6];
    snprintf(strport, sizeof(strport), "%d", port);

    /* получаем возможные адреса для подключения к host, заполняя res */
    int err = getaddrinfo(host, strport, &hints, &res);
    if (err)
        return -1;

    int sockfd = -1;
    struct addrinfo *p;
    /* timeval - структура для определения тайм-аута подключения */
    struct timeval timeout;
    timeout.tv_sec = 3;  // секунды
    timeout.tv_usec = 0; // микросекунды
    /* проходим по каждому из возможных адресов и пробуем подключиться */
    for (p = res; p; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;
        /* устанавливаем параметры для connect */
        if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == -1) {
            close(sockfd);
            continue;
        }
        /* успешно */
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == 0)
            break;
        /* провал, пробуем еще раз */
        close(sockfd);
        sockfd = -1;
    }
    freeaddrinfo(res);
    int result = (sockfd >= 0) ? 0 : -1;
    close(sockfd);
    return result;
}
