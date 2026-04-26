#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

/* Программа, описывающая принцип передачи данных посредством скрытых каналов                                */
/* Принцип заключается в создании отправителем временного файла, наличие                                     */
/* которого сообщает получателю, что передается единичный бит, отсувствие файла ознаменует собой нулевой бит */
/* даже не имея общего доступа к одному и тому же файлу, системный вызов stat позволяет узнать о его         */
/* существованни, показывая, что невозможно добиться полной конфидециальности даже при использовании прочих  */
/* средств защиты                                                                                            */

#define PATH "/tmp/.covert"
#define BIT_DELAY_US 10000 /* время задержки отправителя        */
#define POLL_US 5000       /* время задержки опроса получателем */

/* Поток отправителя */
void* sender(void *arg) {
    int c;
    while ((c = getchar()) != EOF) {
        if (c == '1') {
            creat(PATH, 0);
        } else if (c == '0') {
            unlink(PATH);
        }
        usleep(BIT_DELAY_US);
    }
    unlink(PATH);
    return NULL;
}

/* Поток получателя */
void* receiver(void *arg) {
    struct stat st;
    int prev_state = -1;
    while (1) {
        int exists = (stat(PATH, &st) == 0);
        if (exists != prev_state) {
            putchar(exists ? '1' : '0');
            fflush(stdout);
            prev_state = exists;
        }
        usleep(POLL_US);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    pthread_t tid_sender, tid_receiver;

    pthread_create(&tid_sender, NULL, sender, NULL);
    pthread_create(&tid_receiver, NULL, sender, NULL);

    pthread_join(tid_sender, NULL);

    return 0;
}
