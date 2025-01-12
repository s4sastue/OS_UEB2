#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <zmq.h>


#define NUM_ITERATIONS 1000000
#define MSQ_SIZE 1

//#define ZMQ_ADRESS "tcp://localhost:42"
//#define ZMQ_ADRESS "ipc:///tmp/zmq"
#define ZMQ_ADRESS "inproc://zmq"

void *server_func(void *context){
    void *responder = zmq_socket(context, ZMQ_REP);
    zmq_bind(responder, ZMQ_ADRESS);

    char buffer[MSQ_SIZE];
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        zmq_recv(responder, buffer, MSQ_SIZE, 0);
        zmq_send(responder, "", MSQ_SIZE, 0);
    }

    // Schließe den Socket und den Kontext
    zmq_close(responder);

    return NULL;
}

void *client_func(void *context){
    void *requester = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester, ZMQ_ADRESS);

    long latencies[NUM_ITERATIONS];
    struct timespec start, end;
    char buffer[MSQ_SIZE]= {0};

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);

        zmq_send(requester, "", MSQ_SIZE, 0);
        zmq_recv(requester, buffer, MSQ_SIZE, 0);

        clock_gettime(CLOCK_MONOTONIC, &end);
        latencies[i] = (end.tv_sec * 1e9 + end.tv_nsec) - (start.tv_sec * 1e9 + start.tv_nsec);
    }

    zmq_close(requester);

    FILE *csv_file = fopen("latencies_zeroMQ_intraProcess_INPROC.csv", "w");
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

int main() {
    void *context = zmq_ctx_new();

    pthread_t thread1, thread2;

    pthread_create(&thread1, NULL, server_func, context);
    pthread_create(&thread2, NULL, client_func, context);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    zmq_ctx_destroy(context);

    return 0;
}