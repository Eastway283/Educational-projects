#include <stdio.h>
#include <unistd.h>

/* внешний массив переменных */
extern char **environ;

/* env: вывод списка всех текущих переменных среды */ 
int main(void)
{
    for (char **env = environ; *env != NULL; env++)
            puts(*env);
    return 0;
}
