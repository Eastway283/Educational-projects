#include <stdio.h>
#include <stdlib.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

// атомарная переменная для изменения флага работы
volatile sig_atomic_t running = 1;

// функция для обработчика сигналов
void sighandler(int sig) {
    if (sig == SIGTERM || sig == SIGINT)
        running = 0;
}

// простой демон, который пишет в системный лог сообщение
// прекращает работу после получения сигналов
void mydaemon(void) {
    // создаем дочерний процесс
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    // завершаем родителя, чтобы избавиться от лидерства
    if (pid != 0)
        exit(EXIT_SUCCESS);

    // создаем новую сессию, где демон будет автономен
    if (setsid() < 0) {
        perror("setsid");
        exit(EXIT_FAILURE);
    }

    // повторный форк, чтобы гарантироват, что лидерства нету
    pid = fork();
    if (pid < 0) {
        perror("second fork");
        exit(EXIT_FAILURE);
    }
    if (pid != 0)
        exit(EXIT_SUCCESS);

    // переходим в корневой каталог, чтобы не блокировать ФС
    if (chdir("/") < 0) {
        perror("chdir");
        exit(EXIT_FAILURE);
    }
    // отключаем маску создания файлов
    umask(0);

    // закрываем стандартные потоки
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // переназначаем стандарные потоки, чтобы не вызывать ошибок
    open("/dev/null", O_RDWR);
    dup(0); // stdout == dev/null
    dup(0); // stderr == dev/null
}

int main(void) {

    // настраиваем и запускаем демона
    mydaemon();

    // открываем системный лог от имени mydaemon
    openlog("mydaemon", LOG_PID, LOG_DAEMON);

    // Устанавливаем обработчики сигналов
    struct sigaction sa;
    sa.sa_handler = sighandler;
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    // Заходим в основной цикл и пишем логи
    while (running) {
        syslog(LOG_INFO, "Daemon is running, pid = %d", getpid());
        sleep(5);
    }

    syslog(LOG_INFO, "Daemon is stopped");
    closelog();
    return 0;
}

