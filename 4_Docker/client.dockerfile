FROM debian:bullseye-slim

RUN apt-get update && apt-get install -y \
    build-essential \
    libzmq3-dev

COPY zmq_client.c /zmq_client.c

RUN gcc -Wall -g zmq_client.c -lzmq -o zmq_client

CMD ["./zmq_client"]
