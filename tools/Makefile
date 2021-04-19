CC=gcc

all: ubitx_controller ubitx_client

ubitx_controller: ubitx_controller.c serial.c shm.c ubitx_controller.h serial.h shm.h
	$(CC) -Wall -lpthread ubitx_controller.c serial.c shm.c -o ubitx_controller

ubitx_client: ubitx_client.c shm.c ubitx_controller.h shm.h
	$(CC) -Wall -lpthread ubitx_client.c shm.c -o ubitx_client

.PHONY: clean
clean:
	rm -f *.o ubitx_client ubitx_controller
