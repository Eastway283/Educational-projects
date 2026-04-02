#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_REQUESTS 1000
#define MAX_CYL 1000

int compare_ints(const void *a, const void *b)
{
    int ia = *(const int *)a;
    int ib = *(const int *)b;

    return (ia > ib) - (ia < ib);
}

int fcfs(int *requests, int n, int start)
{
    int total = 0;
    int current = start;
    for (int i = 0; i < n; i++)
    {
        total += abs(requests[i] - current);
        current = requests[i];
    }
    return total;
}

int sstf(int *requests, int n, int start)
{
    int *served = calloc(n, sizeof(int));
    int total = 0;
    int current = start;
    int served_count = 0;
    while (served_count < n) {
        int best = -1;
        int best_dist = MAX_CYL + 1;
        for (int i = 0; i < n; i++) {
            if (!served[i]) {
                int dist = abs(requests[i] - current);
                if (dist < best_dist) {
                    best_dist = dist;
                    best = i;
                }
            }
        }
        total += best_dist;
        current = requests[best];
        served[best] = 1;
        served_count++;
    }
    free(served);
    return total;
}

int scan(int *request, int n, int start)
{
    int *sorted = (int *)malloc(n * sizeof(int));
    memcpy(sorted, request, n * sizeof(int));
    qsort(sorted, n, sizeof(int), compare_ints);

    int total = 0;
    int current = start;

    for (int i = 0; i < n; i++)
        if (sorted[i] >= current) {
            total += sorted[i] - current;
            current = sorted[i];
        }

    for (int i = n - 1; i >= 0; i--)
        if (sorted[i] < current) {
            total += current - sorted[i];
            current = sorted[i];
        }

    free(sorted);
    return total;
}

int main(int argc, char **argv)
{
    int n = 1000;
    if (argc > 1)
        n = atoi(argv[1]);
    if (n > MAX_REQUESTS)
        n = MAX_REQUESTS;

    int *requests = (int *)malloc(n * sizeof(int));
    srand(time(NULL));
    for (int i = 0; i < n; i++)
        requests[i] = rand() % MAX_CYL;

    int start = n / 2;
    int f = fcfs(requests, n, start);
    int s = sstf(requests, n, start);
    int sc = scan(requests, n, start);

    printf("Количество запросов: %d\n", n);
    printf("FCFS: %d цилиндров\n", f);
    printf("SSTF: %d цилиндров\n", s);
    printf("SCAN: %d цилиндров\n", sc);
    free(requests);

    return 0;
}
