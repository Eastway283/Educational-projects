#include <asm-generic/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>

#define BUFSIZE 1024

void usage(const char *prog);

int main(int argc, char **argv)
{
    if (argc < 3) usage(argv[0]);

    int listen_fd = -1, sock_fd = -1;
    struct sockaddr_in addr;
    int port;

    if (strcmp(argv[1], "-s") == 0) { //Server mode
        port = atoi(argv[2]);
        if (port <= 0) {
            fprintf(stderr, "%s: invalid port %s\n", argv[0], argv[2]);
            exit(EXIT_FAILURE);
        }
        
        if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        int opt = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("bind"); close(listen_fd); exit(EXIT_FAILURE);
        }

        if (listen(listen_fd, 1) < 0) {
            perror("listen"); close(listen_fd); exit(EXIT_FAILURE);
        }

        printf("Waiting for connection on port %d...\n", port);
        socklen_t addrlen = sizeof(addr);
        sock_fd = accept(listen_fd, (struct sockaddr *)&addr, &addrlen);
        if (sock_fd < 0) {
            perror("accept"); close(listen_fd); exit(EXIT_FAILURE);
        }

        printf("Connected to %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        close(listen_fd); // Больше не нужен
    } else if (strcmp(argv[1], "-c") == 0) { //Client mode
        if (argc < 4)
            usage(argv[0]);
        const char *server_ip = argv[2];
        port = atoi(argv[3]);
        if (port <= 0) {
            fprintf(stderr, "%s: invalid port %s\n", argv[0], argv[3]);
            exit(EXIT_FAILURE);
        }
        if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket");
            exit(EXIT_FAILURE);
        }
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        if (inet_pton(AF_INET, server_ip, &addr.sin_addr) <= 0) {
            perror("inet_pton"); close(sock_fd); exit(EXIT_FAILURE);
        }
        if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("connect"); close(sock_fd); exit(EXIT_FAILURE);
        }
        printf("Connected to %s:%d\n", server_ip, port);
    } else {
        usage(argv[0]);
    }

    // Подключение установлено, можно начинать обмен сообщениями
    printf("Chat started. Type your message (Ctrl + D on empty line to exit)\n");

    fd_set read_fds;
    char buff[BUFSIZE];
    int max_fd = (sock_fd > STDIN_FILENO) ? sock_fd : STDIN_FILENO;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(sock_fd, &read_fds);

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        // Проверяем, пришли ли данные от собеседника
        if (FD_ISSET(sock_fd, &read_fds)) {
            int n = recv(sock_fd, buff, BUFSIZE - 1, 0);
            if (n <= 0) {
                fprintf(stdout, "\nConnection closed by peer\n");
                break;
            }
            buff[n] = '\0';
            printf("\r[Peer] %s\n", buff);
            // После вывода сообщения нужно перепечатать приглашение
            printf("You: ");
            fflush(stdout);
        }
        // Проверяем, ввел ли пользователь сообщение
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            printf("You: ");
            fflush(stdout);
            if (fgets(buff, BUFSIZE, stdin) == NULL) {
                printf("\nClosing connection\n");
                break;
            }
            // Убираем перевод строки, если есть
            buff[strcspn(buff, "\n")] = '\0';
            // Отправляем сообщение
            int len = strlen(buff);
            if (len > 0) {
                if (send(sock_fd, buff, len, 0) < 0) {
                    perror("sent");
                    break;
                }
            }
        }
    }


    close(sock_fd);
    return 0;
}

void usage(const char *prog) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "Server mode: %s -s <port>\n", prog);
    fprintf(stderr, "Client mode: %s -c <server_ip> <port>\n", prog);
    exit(EXIT_FAILURE);
}
