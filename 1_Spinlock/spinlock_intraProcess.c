#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#define NUM_ITERATIONS 1000000

volatile int lock1 = 0;
volatile int lock2 = 0;
pthread_t thread1, thread2;


void *thread1_func(void *arg) {
    long latencies[NUM_ITERATIONS];
    struct timespec start, end;

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);

        pthread_spin_lock(&lock1);
        pthread_spin_unlock(&lock2);

        clock_gettime(CLOCK_MONOTONIC, &end);

        latencies[i] = (end.tv_sec * 1e9 + end.tv_nsec) - (start.tv_sec * 1e9 + start.tv_nsec);;
    }

    FILE *csv_file = fopen("latencies_spinlock_intraProcess.csv", "w");
    if (csv_file == NULL) {
        perror("Fehler beim Öffnen der Datei");
        return NULL;
    }

    fprintf(csv_file, "Latenz(ns)\n");
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        fprintf(csv_file, "%ld\n", latencies[i]);
    }
    fclose(csv_file);

    return NULL;
}


void *thread2_func(void *arg) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        pthread_spin_lock(&lock2);
        pthread_spin_unlock(&lock1);

    }

    return NULL;
}

int main() {
    pthread_spin_init(&lockA, PTHREAD_PROCESS_PRIVATE);
    pthread_spin_init(&lockB, PTHREAD_PROCESS_PRIVATE);

    pthread_create(&thread1, NULL, thread1_func, NULL);
    pthread_create(&thread2, NULL, thread2_func, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}
