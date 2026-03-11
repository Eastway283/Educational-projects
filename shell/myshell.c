#define _POSIX_C_SOURCE 200112L //для специальных POSIX функций

#include <linux/limits.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "builtins.h"

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"

#define MAX_LINE_SIZE 1000
#define MAX_ARGV_SIZE 64

char cwd[PATH_MAX];

int type_promt(void);

int get_command(char*, size_t);

int main(void)
{
    //TODO: add PATH to my utils
    char line[MAX_LINE_SIZE];
    char *argv[MAX_ARGV_SIZE], *token;
    int argc;
    while (1) {
        if (type_promt() != 0) {
            perror("type_promt error");
            exit(1);
        }
        if (get_command(line, sizeof(line)) == EOF) {
            putchar('\n');
            break;
        }
        //Убираем символ новой строки, который добавляет fgets
        line[strcspn(line, "\n")] = '\0';
        //Разбиваем строку на токены
        argc = 0;
        token = strtok(line, " \t"); //Пробелы и табуляции как разделители
        while (token != NULL && argc < MAX_ARGV_SIZE - 1) {
            argv[argc++] = token;
            token = strtok(NULL, " \t");
        }
        argv[argc] = NULL; //Требуется для exec
        if (argc == 0)
            continue;

        //Обработка встроенных комманд
        int ret = exec_builtin(argv);
        if (ret == BUILTINS_EXIT)
            break;
        if (ret != BUILTINS_NOT_FOUND)
            continue;

        //Вызов дочерних процессов
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork error");
            continue;
        } else if (pid == 0) {  //дочерний процесс
            execvp(argv[0], argv);
            //если вернулся, то ошибка
            fprintf(stderr, "%s: command not found\n", argv[0]);
            exit(1);
        } else { //родительский процесс
            int status;
            waitpid(pid, &status, 0);
        }
    }

    return 0;
}

int type_promt(void)
{
    static char hostname[256];
    static char username[256];
    static int initialized = 0;
    if (!initialized) {
        //пользователь
        const char *user = getenv("USER");
        if (user != NULL) {
            strncpy(username, user, sizeof(username) - 1);
            username[sizeof(username) - 1] = '\0';
        }
        //хост
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            hostname[sizeof(hostname) - 1] = '\0';
            //обрезаем до первой точки
            char *dot = strchr(hostname, '.');
            if (dot)
                *dot = '\0';
        } else {
            strcpy(hostname, "unknown");
        }
        initialized = 1;
    }
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf(COLOR_GREEN "%s@%s" COLOR_RESET ":" COLOR_CYAN "~%s" COLOR_RESET "$ ", username, hostname, cwd);
        return 0;
    }
    else
       return -1;
}

int get_command(char *line, size_t maxlen)
{
    if (fgets(line, maxlen - 1, stdin) == NULL)
        return EOF;
    return 0;
}
