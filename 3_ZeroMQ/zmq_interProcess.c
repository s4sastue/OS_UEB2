#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <zmq.h>


#define NUM_ITERATIONS 1000000
#define MSQ_SIZE 1

#define ZMQ_ADRESS "tcp://localhost:42"
//#define ZMQ_ADRESS "ipc:///tmp/zmq"

void *server_func(void *arg){
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_REP);
    zmq_bind(responder, ZMQ_ADRESS);

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        zmq_recv(responder, "", MSQ_SIZE, 0);
        zmq_send(responder, "", MSQ_SIZE, 0);
    }

    zmq_close(responder);
    zmq_ctx_destroy(context);

    return NULL;
}

void *client_func(void *arg){
    void *context = zmq_ctx_new();
    void *requester = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester, ZMQ_ADRESS);

    long latencies[NUM_ITERATIONS];
    struct timespec start, end;

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);

        zmq_send(requester, "", MSQ_SIZE, 0);
        zmq_recv(requester, "", MSQ_SIZE, 0);

        clock_gettime(CLOCK_MONOTONIC, &end);
        latencies[i] = (end.tv_sec * 1e9 + end.tv_nsec) - (start.tv_sec * 1e9 + start.tv_nsec);
    }

    zmq_close(requester);
    zmq_ctx_destroy(context);

    FILE *csv_file = fopen("latencies_zeroMQ_interProcess_TCP.csv", "w");
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
    pid_t pid = fork();
    if (pid == 0) {
        server_func(NULL);
    } else if (pid > 0) {
        client_func(NULL);
    }

    return 0;
}