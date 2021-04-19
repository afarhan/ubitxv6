CC=gcc
PREFIX=/usr
CFLAGS=-g -Wall -pedantic -std=gnu11 -pthread -fstack-protector

all: ubitx_controller ubitx_client

ubitx_controller: ubitx_controller.c serial.c shm.c ubitx_controller.h serial.h shm.h
	$(CC) $(CFLAGS) ubitx_controller.c serial.c shm.c -o ubitx_controller

ubitx_client: ubitx_client.c shm.c ubitx_controller.h shm.h
	$(CC) $(CFLAGS) ubitx_client.c shm.c -o ubitx_client

.PHONY: clean install
clean:
	rm -f *.o ubitx_client ubitx_controller

install: ubitx_controller ubitx_client
	install ubitx_client $(PREFIX)/bin
	install ubitx_controller $(PREFIX)/bin
