#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>


#define NUM_ITERATIONS 1000000
#define SHM_NAME "/shared_mem"

pthread_t thread1, thread2;

typedef struct {
    int lock1;
    int lock2;
} locks;


void *thread1_func(void *arg) {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open sender");
        exit(EXIT_FAILURE);
    }

    locks* lock_area = mmap(NULL, sizeof(locks), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (lock_area == MAP_FAILED) {
        perror("mmap sender");
        exit(EXIT_FAILURE);
    }

    long latencies[NUM_ITERATIONS];
    struct timespec start, end;

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);

        pthread_spin_lock(&lock_area->lock1);
        pthread_spin_unlock(&lock_area->lock2);

        clock_gettime(CLOCK_MONOTONIC, &end);

        latencies[i] = (end.tv_sec * 1e9 + end.tv_nsec) - (start.tv_sec * 1e9 + start.tv_nsec);;
    }


    FILE *csv_file = fopen("latencies_spinlock_intraProcess_SharedMemory.csv", "w");
    if (csv_file == NULL) {
        perror("Fehler beim Öffnen der Datei");
        return NULL;
    }

    fprintf(csv_file, "Latenz(ns)\n");
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        fprintf(csv_file, "%ld\n", latencies[i]);
    }
    fclose(csv_file);

}


void *thread2_func(void *arg) {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open sender");
        exit(EXIT_FAILURE);
    }

    locks* lock_area = mmap(NULL, sizeof(locks), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (lock_area == MAP_FAILED) {
        perror("mmap sender");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        pthread_spin_lock(&lock_area->lock2);
        pthread_spin_unlock(&lock_area->lock1);
    }

    return NULL;
}

int main() {
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open main");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, sizeof(locks)) == -1) {
        perror("ftruncate main");
        exit(EXIT_FAILURE);
    }

    locks* lock_area = mmap(NULL, sizeof(locks), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (lock_area == MAP_FAILED) {
        perror("mmap main");
        exit(EXIT_FAILURE);
    }

    pthread_spin_init(&lock_area->lock1, PTHREAD_PROCESS_PRIVATE);
    pthread_spin_init(&lock_area->lock2, PTHREAD_PROCESS_PRIVATE);

    pthread_create(&thread1, NULL, thread1_func, NULL);
    pthread_create(&thread2, NULL, thread2_func, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    shm_unlink(SHM_NAME);

    return 0;
}
