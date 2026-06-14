#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rle.h"
#include <unistd.h>
#include <fcntl.h>

#define HEADER "RLE" // заголовок
#define HEADERSIZ 3  // размер заголовка в байтах
#define EXT ".rle"   // расширение файла
#define EXTSIZ 4     // размер расширения в байтах

/* Режим работы */
#define ENCODE 1 // сжатие
#define DECODE 2 // распаковка

/* Права создания файла: rw-r--r-- */
#define PERMS 0644

unsigned long fsize(int fd) {
    unsigned long temp;
    temp = (unsigned long)lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    return temp;
}

void eprint(const char *msg) {
    write(STDERR_FILENO, msg, strlen(msg));
    exit(EXIT_FAILURE);
}

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

    if (argc != 3)
        eprint("Usage: ./rle -[e,d] file\n");

    unsigned char flag = 0;
    if (strcmp(argv[1], "-e") == 0)
        flag = ENCODE;
    else if (strcmp(argv[1], "-d") == 0)
        flag = DECODE;
    else {
        write(STDERR_FILENO, "rle: illegal option\n", strlen("rle: illegal option\n"));
        eprint("Usage: ./rle -[e,d] file\n");
    }

    char *i_name, *o_name;
    i_name = argv[2];
    o_name = NULL;
    int i_fd, o_fd;

    if ((i_fd = open(i_name, O_RDONLY)) < 0)
        error("open");

    if (flag == ENCODE) {
        o_name = (char *)malloc(strlen(i_name) + EXTSIZ + 1);
        if (!o_name)
            error("malloc");
        // формирование имени выходного файла
        sprintf(o_name, "%s%s", i_name, EXT);
        if ((o_fd = open(o_name, O_WRONLY | O_CREAT | O_TRUNC, PERMS)) < 0)
            error("open");

        // size - размер исходного файла в байтах
        unsigned long size = fsize(i_fd);
        unsigned char *buff = (unsigned char *)malloc(size);
        if (!buff)
            error("malloc");
        // чтение файла в буффер
        if ((read(i_fd, buff, size)) != size)
            error("read");
        close(i_fd);

        size_t osize;
        unsigned char *encoded = rle_encode(buff, size, &osize);
        if (!encoded)
            error("rle_encode fail");
        free(buff);

        // запись заголовка для последующей распаковки
        if ((write(o_fd, HEADER, HEADERSIZ) != HEADERSIZ))
            error("write");
        // запись сжатой последовательности
        if ((write(o_fd, encoded, osize)) != osize)
            error("write");

        close(o_fd);
        free(encoded);
        free(o_name);

    } else {
        // проверка на соответствие расширению
        const char *ext_pos = strstr(i_name, EXT);
        if (!ext_pos || strcmp(ext_pos, EXT) != 0)
            eprint("rle: file must have .rle extension\n");

        size_t len = strlen(i_name) - EXTSIZ;
        o_name = (char *)malloc(len + 1);
        if (!o_name)
            error("malloc");

        memcpy(o_name, i_name, len);
        o_name[len] = '\0';

        if ((o_fd = open(o_name, O_WRONLY | O_CREAT | O_TRUNC, PERMS)) < 0)
            error("open");

        unsigned long size = fsize(i_fd);
        if (size < 3) // Файл не содержит даже заголовка
            eprint("rle: file too small to be RLE encoded");

        unsigned char *buff = (unsigned char *)malloc(size);
        if (!buff)
            error("malloc");

        if ((read(i_fd, buff, size)) != size)
            error("write");
        close(i_fd);

        // проверка на наличие заголовка
        if (memcmp(buff, HEADER, HEADERSIZ) != 0)
            eprint("rle: invalid .rle file (missing header)");

        // пропуск заголовка
        unsigned char *compressed = buff + HEADERSIZ;
        size -= HEADERSIZ;
        size_t dec_size;
        unsigned char *decoded = rle_decode(compressed, size, &dec_size);
        if (!decoded)
            error("rle_decode fail");

        free(buff);
        if ((write(o_fd, decoded, dec_size)) != dec_size)
            error("write");

        free(decoded);
        free(o_name);
        close(o_fd);
    }

    return 0;
}
