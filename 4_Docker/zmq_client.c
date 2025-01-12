#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_ITERATIONS 1000000
#define MSQ_SIZE 1
#define ZMQ_ADRESS "tcp://server:4242"

int main() {
    void *context = zmq_ctx_new();
    void *requester = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester, ZMQ_ADRESS);

    long latencies[NUM_ITERATIONS];
    struct timespec start, end;
    char buffer[MSQ_SIZE] = {0};

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);

        zmq_send(requester, "", MSQ_SIZE, 0);
        zmq_recv(requester, buffer, MSQ_SIZE, 0);

        clock_gettime(CLOCK_MONOTONIC, &end);
        latencies[i] = (end.tv_sec * 1e9 + end.tv_nsec) - (start.tv_sec * 1e9 + start.tv_nsec);
    }

    zmq_close(requester);
    zmq_ctx_destroy(context);

    FILE *csv_file = fopen("/res/latencies_docker_via_zeroMQ.csv", "w");
    if (csv_file == NULL) {
        perror("Fehler beim Öffnen der Datei");
        return 0;
    }

    fprintf(csv_file, "Latenz(ns)\n");
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        fprintf(csv_file, "%ld\n", latencies[i]);
    }
    fclose(csv_file);

    zmq_close(requester);
    zmq_ctx_destroy(context);

    return 0;
}
