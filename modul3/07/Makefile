CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=gnu11 -Iinclude

SRC_DIR = src
BIN_DIR = bin

TCP_SOURCES = $(SRC_DIR)/tcp_main.c $(SRC_DIR)/tcp_client.c $(SRC_DIR)/tcp_server.c
TCP_TARGET = $(BIN_DIR)/tcpchat

all: $(TCP_TARGET)

$(TCP_TARGET): $(TCP_SOURCES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(BIN_DIR)

run-server: $(TCP_TARGET)
	$(TCP_TARGET) server

run-client: $(TCP_TARGET)
	$(TCP_TARGET) client 127.0.0.1

.PHONY: all clean run-server run-client