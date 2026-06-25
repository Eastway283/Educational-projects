#include "server.h"
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

char *root;
int port;

int main(int argc, char **argv) {

    if (argc > 3) {
        fprintf(stderr, "%s: too many arguments\n", argv[0]);
        fprintf(stderr, "Usage: %s [port] [root]\n", argv[0]);
        return 1;
    } else if (argc > 1) {
        port = atoi(argv[1]);
        if (port < 1 || port > MAXPORT) {
            fprintf(stderr, "%s: forbidden port value %s\n", argv[0], argv[1]);
            return 1;
        }
        /* нет корня */
        if (argc == 2)
            root = ROOT;
        /* есть корень */
        else
            root = argv[2];
    } else { /* запуск без аргументов */
        port = PORT;
        root = ROOT;
    }

    /* создаем и начинаем слушать сокет */
    int serverfd = create_server_sock(port);
    if (serverfd < 0) {
        perror("create_server_sock");
        return 1;
    }

    /* объявляем переменные для обработки запросов */
    char buff[BUFSIZ];
    int clientfd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char client_ip[INET_ADDRSTRLEN];
    char fullpath[PATH_MAX];

    /* сообщаем о успешном старте сервера */
    printf("Server started\n");
    printf("Server info:\n");
    printf("root: %s\n", root);
    printf("port: %d\n", port);
    printf("Waiting clients...\n");

    /* заходим в бесконечный цикл и обрабатываем запросы */
    while (1) {

        /* принимаем запрос от клиента */
        clientfd = accept(serverfd, (struct sockaddr *)&addr, &addrlen);
        if (clientfd < 0) {
            perror("accept");
            continue;
        }

        /* преобразуем бинарный адрес в строку и выводим сообщение */
        if (!inet_ntop(AF_INET, &addr.sin_addr, client_ip, INET_ADDRSTRLEN)) {
            perror("inet_ntop");
            close(clientfd);
            continue;
        }
        printf("Client %s:%d connected\n", client_ip, ntohs(addr.sin_port));

        /* читаем запрос от клиента в буфер */
        ssize_t n = read(clientfd, buff, BUFSIZ - 1);
        if (n <= 0) {
            perror("read");
            close(clientfd);
            continue;
        }
        buff[n] = '\0';

        /* обрезаем строку для обработки заголовка */
        char *line_end = strstr(buff, "\r\n");
        if (!line_end) {
            send_error(clientfd, 400);
            close(clientfd);
            continue;
        }
        *line_end = '\0';
        /* парсим метод, путь и версию */
        char method[16], path[512], version[16];
        if (sscanf(buff, "%15s %511s %15s", method, path, version) != 3) {
            send_error(clientfd, 400);
            close(clientfd);
            continue;
        } else if (strcmp(method, "GET") != 0) { /* поддерживаем только GET */
            send_error(clientfd, 405);
            close(clientfd);
            continue;
        }

        /* формируем абсолютный путь для проверки доступа */
        snprintf(fullpath, PATH_MAX, "%s/%s", root, path);
        if (!check_path(fullpath, root)) {
            send_error(clientfd, 403);
            close(clientfd);
            continue;
        }

        /* проверяем наличие файла */
        struct stat st;
        if (stat(fullpath, &st) != 0) {
            send_error(clientfd, 404);
            close(clientfd);
            continue;
        }

        /* проверка на принадлежность к каталогу */
        if (S_ISDIR(st.st_mode)) {
            /* сохраняем исходный путь для листинга */
            char dir_path[PATH_MAX];
            strcpy(dir_path, fullpath);
            /* добавляем индекс к пути и проверяем доступ */
            strcat(fullpath, "/index.html");
            if (!check_path(fullpath, root)) {
                /* index.html недоступен, отсылаем листинг */
                if (!send_dir(clientfd, dir_path))
                    send_error(clientfd, 500);
                close(clientfd);
                continue;
            }
            /* проверяем существование index.html */
            struct stat st;
            if (stat(fullpath, &st) != 0) {
                /* не существует, отправляем листинг */
                if (!send_dir(clientfd, dir_path))
                    send_error(clientfd, 500);
                close(clientfd);
                continue;
            }
        }

        /* отправляем файл и закрываем соединение */
        if (!send_file(clientfd, fullpath)) {
            send_error(clientfd, 500);
            close(clientfd);
            continue;
        }
        close(clientfd);
    }

    return 0;
}
