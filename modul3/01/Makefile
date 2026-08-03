CC = gcc
# Изменено c99 на gnu17 для поддержки современных стандартов POSIX
CFLAGS = -Wall -Wextra -std=gnu17
TARGET = copier

SRCS = main.c parent.c child.c protocol.c pipe_utils.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) *.copy my_fifo