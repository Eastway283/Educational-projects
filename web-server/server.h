#ifndef SERVER_H
#define SERVER_H

#include <stddef.h>

/* корень сервера и порт */
#define ROOT "public/"
#define PORT 8080 // TODO: заменить на 80, когда все протестируем
#define MAXPORT 65535

/* количество обслуживаемых клиентов */
#define USER_COUNT 10

/* структура для оптимизации получения MIME-type */
typedef struct mime_type {
    const char *ext;
    const char *type;
} mime_type;

/* структура для оптимизации обработки ошибок */
typedef struct err_type {
    int code;
    const char *text;
} err_type;

/* status_text - возвращает указатель на описание ошибки.
 * если код не найден - Internal Server Error */
const char *status_text(int code);

/* create_server_sock - создает, биндит и начинает слушать
 * возвращает дескриптор сокета или -1 в случае ошибки */
int create_server_sock(int port);

/* get_mime_type - определяет content-type по расширению файла
 * path - путь к файлу. Возвращает mime, NULL в случае ошибки */
const char *get_mime_type(const char *path);

/* check_path - проверяет, что абсолютное имя файла fullpath не выходит за корень root
 * ненулевое число в случае успеха, 0 в случае ошибки или провала проверки */
int check_path(const char *fullpath, const char *root);

/* send_error - отправляет HTTP-ответ об ошибке
 * ненулевое число в случае успеха, 0 в случае ошибки отправки */
int send_error(int client_fd, int status_code);

/* send_file - открывает файл, читает и отправляет вместе с заголовком
 * ненулевое число в случае успеха, 0 в случае ошибки */
int send_file(int client_fd, const char *path);

/* send_dir - отправляет клиенту список список файлов в текущем каталоге
 * ненулевое число в случае успеха, 0 в случае ошибки */
int send_dir(int client_fd, const char *path);

#endif /* SERVER_H */
