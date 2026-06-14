#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rle.h"

#define HEADER "RLE" // заголовок
#define EXT ".rle"   // расширение файла

/* Режим работы */
#define ENCODE 1 // сжатие
#define DECODE 2 // распаковка

unsigned long fsize(FILE *f) {
    unsigned long temp;
    fseek(f, 0, SEEK_END);
    temp = ftell(f);
    fseek(f, 0, SEEK_SET);
    return temp;
}

int main(int argc, char **argv) {

    if (argc != 3) {
        fprintf(stderr, "Usage: %s -[e,d] file\n", argv[0]);
        exit(EXIT_FAILURE);   // добавлен выход, чтобы не продолжать
    }

    unsigned char flag = 0;
    if (strcmp(argv[1], "-e") == 0)
        flag = ENCODE;
    else if (strcmp(argv[1], "-d") == 0)
        flag = DECODE;
    else {
        fprintf(stderr, "%s: illegal option %s\n", argv[0], argv[1]);
        fprintf(stderr, "Usage: %s -[e,d] file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *i_name, *o_name;
    i_name = argv[2];
    o_name = NULL;
    FILE *i_file, *o_file;

    if (!(i_file = fopen(i_name, "rb"))) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    if (flag == ENCODE) {
        o_name = (char *)malloc(strlen(i_name) + strlen(EXT) + 1);
        if (!o_name) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        // формирование имени выходного файла
        sprintf(o_name, "%s%s", i_name, EXT);

        if (!(o_file = fopen(o_name, "wb"))) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        // size - размер исходного файла в байтах
        unsigned long size = fsize(i_file);
        unsigned char *buff = (unsigned char *)malloc(size);
        if (!buff) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        // чтение файла в буффер
        if ((fread(buff, 1, size, i_file)) != size) {
            perror("fread");
            exit(EXIT_FAILURE);
        }
        fclose(i_file);

        size_t osize;
        unsigned char *encoded = rle_encode(buff, size, &osize);
        if (!encoded) {
            perror("rle_encode fail");
            exit(EXIT_FAILURE);
        }
        free(buff);

        // запись заголовка для последующей распаковки
        if ((fwrite(HEADER, 1, strlen(HEADER), o_file)) != strlen(HEADER)) {
            perror("fwrite");
            exit(EXIT_FAILURE);
        }
        // запись сжатой последовательности
        if ((fwrite(encoded, 1, osize, o_file)) != osize) {
            perror("fwtire");
            exit(EXIT_FAILURE);
        }

        fclose(o_file);
        free(encoded);
        free(o_name);

    } else {
        // проверка на соответствие расширению
        const char *ext_pos = strstr(i_name, EXT);
        if (!ext_pos) {
            fprintf(stderr, "%s: file must have .rle extension\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        size_t len = strlen(i_name) - strlen(EXT);
        o_name = (char *)malloc(len + 1);
        if (!o_name) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strncpy(o_name, i_name, len);
        o_name[len] = '\0';

        if (!(o_file = fopen(o_name, "wb"))) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        unsigned long size = fsize(i_file);
        if (size < 3) { // Файл не содержит даже заголовка
            fprintf(stderr, "%s: file too small to be RLE encoded\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        unsigned char *buff = (unsigned char *)malloc(size);
        if (!buff) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        if (fread(buff, 1, size, i_file) != size) {
            perror("fread");
            exit(EXIT_FAILURE);
        }
        fclose(i_file);

        // проверка на наличие заголовка
        if (memcmp(buff, HEADER, strlen(HEADER)) != 0) {
            fprintf(stderr, "%s: invalid .rle file (missing header)\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        // пропуск заголовка
        unsigned char *compressed = buff + strlen(HEADER);
        size -= strlen(HEADER);

        size_t dec_size;
        unsigned char *decoded = rle_decode(compressed, size, &dec_size);
        if (!decoded) {
            perror("rle_decode fail");
            exit(EXIT_FAILURE);
        }
        free(buff);

        if ((fwrite(decoded, 1, dec_size, o_file)) != dec_size) {
            perror("fwrite");
            exit(EXIT_FAILURE);
        }

        free(decoded);
        free(o_name);
        fclose(o_file);
    }
    return 0;
}
