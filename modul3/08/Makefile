CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=gnu11 -Iinclude

SRC_DIR = src
BIN_DIR = bin

SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/raw_capture.c
TARGET = $(BIN_DIR)/raw_capture

all: $(TARGET)

$(TARGET): $(SOURCES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(BIN_DIR)

run-chat: $(TARGET)
	sudo $(TARGET) 1

run-dns: $(TARGET)
	sudo $(TARGET) 2

run-all: $(TARGET)
	sudo $(TARGET) 3

.PHONY: all clean run-chat run-dns run-all