#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_ITERATIONS 1000000
#define MSQ_SIZE 1
#define ZMQ_ADRESS "tcp://*:4242"

int main() {
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_REP);
    zmq_bind(responder, ZMQ_ADRESS);

    char buffer[MSQ_SIZE];
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        zmq_recv(responder, buffer, MSQ_SIZE, 0);
        zmq_send(responder, "", MSQ_SIZE, 0);
    }

    // Schließe den Socket und den Kontext
    zmq_close(responder);
    zmq_ctx_destroy(context);

    return 0;
}
