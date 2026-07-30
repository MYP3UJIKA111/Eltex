CC = gcc
CFLAGS = -D_GNU_SOURCE -Wall -Werror -Wextra -pedantic -std=c11 -g
TARGET = priority_queue

SRCS = main.c priority_queue.c
OBJS = $(SRCS:.c=.o)
DEPS = priority_queue.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean