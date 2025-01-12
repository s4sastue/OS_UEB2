FROM debian:bullseye-slim

RUN apt-get update && apt-get install -y \
    build-essential \
    libzmq3-dev

COPY zmq_server.c /zmq_server.c

RUN gcc -Wall -g zmq_server.c -lzmq -o zmq_server

CMD ["./zmq_server"]
