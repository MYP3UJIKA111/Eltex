CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=gnu11

all: raw_udp_server raw_udp_client

raw_udp_server: raw_udp_server.c raw_udp_common.c raw_udp_common.h
	$(CC) $(CFLAGS) raw_udp_server.c raw_udp_common.c -o raw_udp_server

raw_udp_client: raw_udp_client.c raw_udp_common.c raw_udp_common.h
	$(CC) $(CFLAGS) raw_udp_client.c raw_udp_common.c -o raw_udp_client

clean:
	rm -f raw_udp_server raw_udp_client *.o

.PHONY: all clean