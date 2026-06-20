#ifndef URL_H
#define URL_H

#include <string.h>

#define DEFAULT_SCHEME "http"

/* Характеристики портов */
#define HTTP_DPORT 80
#define HTTPS_DPORT 443
#define MAX_PORT 65535

#define DEFAULT_PATH "/"

/* Размер кольцевого буффера */
#define BUFFSIZE 10

/* 
 * url_info - структура, хранящая URL в виде отдельных полей
 * используется функцией parse_url для удобного использования 
 * в сетевых функциях предполагаемая структура URL:
 * scheme://host:port/path 
*/
typedef struct {
    char scheme[16];
    char host[256];
    int port;
    char path[512];
} url_info;

/* 
 * parse_url - разбивает URL адрес на токены, заполняя *urlin из url
 * при отсувствии какой-либо части URL заполняет его DEFAULT_* 
 * при успехе возвращает 0, иначе ненулевое значение 
*/
int parse_url(url_info *urlin, const char *url);

/* 
 * buffer_t - структура описывающая кольцевой буффер,
 * состоящий из url_info. Размер буффера фиксирован
 * индексация происходит за счет адресной арифметики
 * требует инициализации перед использованием
 */
typedef struct {
    url_info slots[BUFFSIZE];
    url_info *head;
    url_info *tail;
    size_t count;
} buffer_t;

/* 
 * initBuffer - инициализирует buf, устанавливая
 * count = 0, голова и хвост указывают на начало slots
*/
void initBuffer(buffer_t *buf);

/* 
 * putBuffer - помещает item в buf путем копирования
 * передвигает tail, при достижении slot[SIZE]
 * устанавливает его на начало slot. возвращает 0 в случае успеха
 * -1 - если буффер заполнен
*/
int putBuffer(buffer_t *buf, const url_info *item);

/* 
 * getBuffer - помещает содержимое из буффера в item
 * сдигая head вперед, при достижении slot[SIZE]
 * устанавливает его на начало slot. Возвращает 0 в случае успеха
 * -1 - если буффер пуст
*/
int getBuffer(buffer_t *buf, url_info *item);

/* 
 * connect_to_host - проверяет возможность соединения с host::port 
 * 0 в случае успеха, -1 в случае ошибки
*/
int connect_to_host(const char *host, int port);

#endif // URL_H
