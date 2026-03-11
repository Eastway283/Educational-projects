#include "builtins.h"
#include <stdio.h>
#include <unistd.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <string.h>

//Выход из оболочки
static int builtin_exit(char **argv)
{
    return BUILTINS_EXIT;
}

//cd: переход в другую директорию
static int builtin_cd(char **argv)
{
    if (argv[1] == NULL) {
        const char *home = getenv("HOME");
        if (home == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }
        if (chdir(home) != 0) {
            perror("cd");
            return 1;
        }
    } else if (argv[2] != NULL) {
        fprintf(stderr, "cd: too many arguments\n");
        return 1;
    } else {
        if (chdir(argv[1]) != 0) {
            perror("cd");
            return 1;
        }
    }
    return 0;
}

//pwd: вывод текущей директории
static int builtin_pwd(char **argv)
{
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
        return 0;
    } else {
        perror("pwd");
        return 1;
    }
}

//Статический массив структур:
static const struct {
    const char *name;    //имя команды
    int (*fcn)(char **); //исполняемая ей функция
} builtins[] = {
    {"exit", builtin_exit}, //выход из оболочки
    {"cd", builtin_cd},     //изменение текущей директории
    {"pwd", builtin_pwd},   //вывод текущей директории
    {NULL, NULL}            //заглушка - необходима для обозначения конца встроенных команд
};

//exec_builtin: выволнение встроенной команды, без создания дочерних процессов
//возвращает значение исполняемой функции элемента массива
//в случае, если это не встроенная команда, то подает об этом сигнал
int exec_builtin(char **argv)
{
    if (argv == NULL || argv[0] == NULL)
        return BUILTINS_NOT_FOUND;

    for (int i = 0; builtins[i].name != NULL; i++)
        if (strcmp(argv[0], builtins[i].name) == 0)
            return builtins[i].fcn(argv);

    return BUILTINS_NOT_FOUND;
}
