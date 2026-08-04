CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
LDFLAGS = 

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET = $(BIN_DIR)/broker

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

run-broker: $(TARGET)
	$(TARGET) -b

run-publisher: $(TARGET)
	$(TARGET) -p news

run-subscriber: $(TARGET)
	$(TARGET) -s news

.PHONY: all clean run-broker run-publisher run-subscriber