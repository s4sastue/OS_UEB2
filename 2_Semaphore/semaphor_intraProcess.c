#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

#define NUM_ITERATIONS 1000000

sem_t sem1;
sem_t sem2;


void* thread1_func(void* arg) {
    long latencies[NUM_ITERATIONS];
    struct timespec start, end;

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);

        sem_post(&sem1);
        sem_wait(&sem2);

        clock_gettime(CLOCK_MONOTONIC, &end);

        latencies[i] = (end.tv_sec * 1e9 + end.tv_nsec) - (start.tv_sec * 1e9 + start.tv_nsec);
    }

    FILE* csv_file = fopen("latencies_semaphore_intraProcess.csv", "w");
    if (csv_file == NULL) {
        perror("Datei konnte nicht geöffnet werden");
        return NULL;
    }


    fprintf(csv_file, "Latenz(ns)\n");
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        fprintf(csv_file, "%ld\n", latencies[i]);
    }

    fclose(csv_file);

    return NULL;
}


void* thread2_func(void* arg) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        sem_wait(&sem1);
        sem_post(&sem2);
    }
    return NULL;
}

int main() {
    sem_init(&sem1, 0, 0);
    sem_init(&sem2, 0, 0);

    pthread_t sender, receiver;
    pthread_create(&sender, NULL, thread1_func, NULL);
    pthread_create(&receiver, NULL, thread2_func, NULL);

    pthread_join(sender, NULL);
    pthread_join(receiver, NULL);

    sem_destroy(&sem1);
    sem_destroy(&sem2);

    return 0;
}
