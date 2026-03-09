#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE 128

/* date: вывод текущей даты */
int main(void)
{
    time_t rawtime;
    struct tm *timeinfo;
    char buf[BUFFER_SIZE];
    size_t len;
    const char *format = "%a %b %d %H:%M:%S %Z %Y"; 

    if (time(&rawtime) == (time_t) -1) {
        perror("time");
        return 1;
    }

    timeinfo = localtime(&rawtime);
    if (timeinfo == NULL) {
        perror("localtime");
        return 1;
    }

    len = strftime(buf, BUFFER_SIZE, format, timeinfo);
    if (len == 0) {
        perror("format error");
        return 1;
    }
    printf("%s\n", buf);
    return 0;
}
