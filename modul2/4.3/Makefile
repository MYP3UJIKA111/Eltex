CC = gcc
CFLAGS = -D_GNU_SOURCE -Wall -Werror -Wextra -pedantic -std=c11 -g
TARGET = spravochnik

SRCS = main.c spravochnik.c
OBJS = $(SRCS:.c=.o)
DEPS = spravochnik.h

TEST_TARGET = run_tests

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

test: spravochnik.o test_spravochnik.c
	@echo "Компиляция тестов..."
	$(CC) $(CFLAGS) -o $(TEST_TARGET) test_spravochnik.c spravochnik.o
	@echo "Запуск тестов...\n"
	./$(TEST_TARGET)

clean:
	rm -f $(OBJS) $(TARGET) $(TEST_TARGET) test_spravochnik.o

.PHONY: all clean test