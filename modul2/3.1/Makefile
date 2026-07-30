CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -g -D_GNU_SOURCE
LDFLAGS = 

TARGET = permissions
SRCS = main.c permissions.c
OBJS = $(SRCS:.c=.o)
DEPS = permissions.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

rebuild: clean all

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean rebuild run