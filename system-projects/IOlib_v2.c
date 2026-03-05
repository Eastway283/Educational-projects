
/* Упражнения 8.2 - 8.4 из книги K&R 2-е издание, версия с битовыми полями */
/* Разработано на Linux, переносимость не герантируется                    */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE 1024
#define OPEN_MAX 20

typedef struct iobuf {
    int cnt;                    /* Сколько осталось символов    */
    char *ptr;                  /* Следующая символьная позиция */
    char *base;                 /* Местонахождение буфера       */
    struct {                    /* Режим доступа к файлу:       */
        unsigned int read  : 1; /* чтение                       */
        unsigned int write : 1; /* запись                       */
        unsigned int unbuf : 1; /* без буферизации              */
        unsigned int eof   : 1; /* достигнут конец              */
        unsigned int err   : 1; /* произошла ошибка             */
    } flag;
    int fd;                     /* Дескриптор файла             */
} file;

file iob[OPEN_MAX] = {
    {0, (char *) 0, (char *) 0, {.read = 1, .write = 0, .unbuf = 0, .eof = 0, .err = 0}, 0},
    {0, (char *) 0, (char *) 0, {.read = 0, .write = 1, .unbuf = 0, .eof = 0, .err = 0}, 1},
    {0, (char *) 0, (char *) 0, {.read = 0, .write = 1, .unbuf = 1, .eof = 0, .err = 0}, 2}
};

#define std_in (&iob[0])
#define std_out (&iob[1])
#define std_err (&iob[2])

int fillbuf(file *);
int flushbuf(int, file *);

#define PERMS 0666
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

file *f_open(char *, char *);
int f_close(file *);
int f_flush(file *);
int f_seek(file *, long, int);

int main(int argc, char *argv[])
{
    char *msg = "hello world\n!";
    if (write(STDOUT_FILENO, msg, strlen(msg)) != (size_t) strlen(msg)) {
        fprintf(stderr, "%s: can't write on stdout\n", argv[0]);
        exit(1);
    }

    return 0;
}

int fillbuf(file* fp) // Создание и заполение буфера ввода
{
    int bufsize;

    if (!fp->flag.read || fp->flag.eof || fp->flag.err)
        return EOF;

    bufsize = fp->flag.unbuf ? 1 : BUFSIZE;

    if (fp->base == NULL) /* Буфера еще нет */
        if ((fp->base = (char *)malloc(bufsize)) == NULL)
            return EOF; /* Не удается создать буфер */

    fp->ptr = fp->base;
    fp->cnt = read(fp->fd, fp->ptr, bufsize);
    if (--fp->cnt < 0) { /* Отнимаем, т.к. собираемся вернуть один символ */
        if (fp->cnt == -1)
            fp->flag.eof = 1;
        else
            fp->flag.err = 1;
        fp->cnt = 0;
        return EOF;
    }
    return (unsigned char) *fp->ptr++;
}

int flushbuf(int c, file *fp) // Принудительная запись буфера и его сброс
{
    int bufsize, nwritten;
    if (!fp->flag.write || fp->flag.err)
        return EOF;
    
    bufsize = fp->flag.unbuf ? 1 : BUFSIZE;

    if (fp->base == NULL) { /* Буфера еще нет */ 
        if ((fp->base = (char *)malloc(bufsize)) == NULL)
            return EOF;
        fp->ptr = fp->base;
        fp->cnt = bufsize;
    }

    /* Если буфер не пуст, то записываем его содержимое */
    if (fp->ptr > fp->base) {
        int towrite = fp->ptr - fp->base;
        if ((nwritten = write(fp->fd, fp->base, towrite)) != towrite) {
            fp->flag.err = 1;
            return EOF;
        }
    }

    /* Сбрасываем указатели */
    fp->ptr = fp->base;
    fp->cnt = bufsize - 1;

    /* Помещаем символ в буфер если он не EOF */
    if (c != EOF) {
        *fp->ptr++ = c;
        return (unsigned char) c;
    }

    return 0;
}

file *f_open(char *name, char *mode)
{
    int fd;
    file *fp;
    if (*mode != 'r' && *mode != 'w' && *mode != 'a')
        return NULL;

    for (fp = iob; fp < iob + OPEN_MAX; fp++)
        if (!fp->flag.read && !fp->flag.write)
            break; /* Нашли свободное место */

    if (fp >= iob + OPEN_MAX)
        return NULL; /* Свободного места нет */ 

    if (*mode == 'w')
        fd = creat(name, PERMS);
    else if (*mode == 'a') {
        if ((fd = open(name, O_WRONLY, 0)) == -1)
            fd = creat(name, PERMS);
        lseek(fd, 0L, 2);
    } else
        fd = open(name, O_RDONLY, 0);

    if (fd == -1)
        return NULL; /* Указанное место недоступно */
    fp->fd = fd;
    fp->cnt = 0;
    fp->base = NULL;
    fp->flag.read = (*mode == 'r');
    fp->flag.write = (*mode != 'r');
    fp->flag.unbuf = 0;
    fp->flag.eof = 0;
    fp->flag.err = 0;
    return fp;
}

int f_close(file *fp)
{
    if (fp == NULL)
        return EOF;
    /* Проверяем, открыт ли для чтения */
    if (fp->flag.write) 
        if (flushbuf(EOF, fp) != 0) /* Выводим буфер и очищаем его */
            return EOF;
    /* Закрываем файл и сбрасывам флаги */
    close(fp->fd);
    if (fp->base != NULL)
        free(fp->base);
    fp->fd = -1;
    fp->cnt = 0;
    fp->flag.write = 0;
    fp->flag.read = 0;
    fp->flag.unbuf = 0;
    fp->flag.eof = 0;
    fp->flag.err = 0;
    fp->base = fp->ptr = NULL;
    return 0;
}

int f_flush(file *fp)
{
    int i, status;
    if (fp == NULL) {
        status = 0;
        for (i = 0; i < OPEN_MAX; i++)
            if (iob[i].flag.write)
                if (flushbuf(EOF, &iob[i]) == EOF)
                    status = EOF; /* Записываем ошибку, но продолжаем */
        return status;
    } else {
        if (!fp->flag.write) /* Проверяем, что открыт на запись */
            return EOF;
        return flushbuf(EOF, fp);
    }
}

int f_seek(file *fp, long offset, int origin)
{
    if (fp == NULL)
        return EOF;

    if (!fp->flag.unbuf && fp->base != NULL) {
        if (fp->flag.write) {
            if (flushbuf(EOF, fp) == EOF) /* Сбрасываем буфер записи */
                    return EOF;
        } else  if (fp->flag.read) {
            if (origin == SEEK_CUR) /* Корректируем позицию, с учетом символов в буфере */
                offset -= fp->cnt;
        }
    }

    /* Само перемещение */
    if (lseek(fp->fd, offset, origin) == EOF)
        return EOF;
    /* Сбрасываем флаги */
    fp->cnt = 0;
    fp->ptr = fp->base;
    fp->flag.eof = 0; /* Сбрасываем флаг конца файла */
    return 0;
}
