#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/time.h>
#include <pthread.h>

#define NUM_ITERATIONS 1000000
#define SHM_NAME "/shared_mem"

// Struktur, die die Semaphoren enthält
typedef struct {
    sem_t sem1;  // Semaphore für den Sender
    sem_t sem2;  // Semaphore für den Empfänger
} semaphores;


void* thread1_func() {
    // Öffne das Shared Memory
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open sender");
        exit(EXIT_FAILURE);
    }

    // Mappe das Shared Memory in den Adressraum
    semaphores* sem_area = mmap(NULL, sizeof(semaphores), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sem_area == MAP_FAILED) {
        perror("mmap sender");
        exit(EXIT_FAILURE);
    }

    long latencies[NUM_ITERATIONS];

    struct timespec start, end;
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);


        sem_post(&sem_area->sem1);
        sem_wait(&sem_area->sem2);

        clock_gettime(CLOCK_MONOTONIC, &end);
        
        latencies[i] = (end.tv_sec * 1e9 + end.tv_nsec) - (start.tv_sec * 1e9 + start.tv_nsec);
    }

    FILE* csv_file = fopen("latencies_semaphore_intraProcess_SharedMemory.csv", "w");
    if (csv_file == NULL) {
        perror("Fehler beim Öffnen der Datei");
        return NULL;
    }

    fprintf(csv_file, "Latenz(ns)\n");
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        fprintf(csv_file, "%ld\n", latencies[i]);
    }
    fclose(csv_file);

    // Unmappe das Shared Memory
    munmap(sem_area, sizeof(semaphores));
    close(shm_fd);
}

void* thread2_func() {
    // Öffne das Shared Memory
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open receiver");
        exit(EXIT_FAILURE);
    }

    // Mappe das Shared Memory in den Adressraum
    semaphores* sem_area = mmap(NULL, sizeof(semaphores), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sem_area == MAP_FAILED) {
        perror("mmap receiver");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        sem_wait(&sem_area->sem1);
        sem_post(&sem_area->sem2);
    }

    // Unmappe das Shared Memory
    munmap(sem_area, sizeof(semaphores));
    close(shm_fd);
}

int main() {
    // Shared Memory und Semaphore erstellen
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open main");
        exit(EXIT_FAILURE);
    }

    // Lege die Größe des Shared Memory fest
    if (ftruncate(shm_fd, sizeof(semaphores)) == -1) {
        perror("ftruncate main");
        exit(EXIT_FAILURE);
    }

    // Mappe das Shared Memory in den Adressraum
    semaphores* sem_area = mmap(NULL, sizeof(semaphores), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sem_area == MAP_FAILED) {
        perror("mmap main");
        exit(EXIT_FAILURE);
    }

    // Initialisiere die Semaphore
    if (sem_init(&sem_area->sem1, 1, 0) == -1) {
        perror("sem_init sem1");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&sem_area->sem2, 1, 0) == -1) {
        perror("sem_init sem2");
        exit(EXIT_FAILURE);
    }

    pthread_t sender, receiver;
    pthread_create(&sender, NULL, thread1_func, NULL);
    pthread_create(&receiver, NULL, thread2_func, NULL);

    // Warten auf Threads
    pthread_join(sender, NULL);
    pthread_join(receiver, NULL);

    // Semaphore und Shared Memory bereinigen
    sem_destroy(&sem_area->sem1);
    sem_destroy(&sem_area->sem2);
    shm_unlink(SHM_NAME);

    return 0;
}

