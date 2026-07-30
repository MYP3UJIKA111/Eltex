CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -g
LDFLAGS = 

TARGET = ip_simulator
SRCS = main.c ip_simulator.c
OBJS = $(SRCS:.c=.o)
DEPS = ip_simulator.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

rebuild: clean all

run: $(TARGET)
	./$(TARGET) 192.168.1.1 24 10

.PHONY: all clean rebuild run