CC = gcc
CFLAGS = -D_GNU_SOURCE -Wall -Werror -Wextra -pedantic -std=c11 -g
TARGET = spravochnik
LIB_NAME = libspravochnik.a

SRCS = main.c
LIB_SRCS = spravochnik.c
OBJS = $(SRCS:.c=.o)
LIB_OBJS = $(LIB_SRCS:.c=.o)
DEPS = spravochnik.h

TEST_TARGET = run_tests

all: $(TARGET)

# Создание статической библиотеки
$(LIB_NAME): $(LIB_OBJS)
	@echo "Создание статической библиотеки $(LIB_NAME)..."
	ar rc $(LIB_NAME) $(LIB_OBJS)
	ranlib $(LIB_NAME)
	@echo "Библиотека успешно создана!"

# Компиляция объектных файлов библиотеки
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Линковка исполняемого файла с библиотекой
$(TARGET): $(OBJS) $(LIB_NAME)
	@echo "Компоновка с библиотекой $(LIB_NAME)..."
	$(CC) $(CFLAGS) -o $@ $(OBJS) -L. -lspravochnik

# Компиляция тестов
test: $(LIB_NAME) test_spravochnik.c
	@echo "Компиляция тестов..."
	$(CC) $(CFLAGS) -o $(TEST_TARGET) test_spravochnik.c -L. -lspravochnik
	@echo "Запуск тестов...\n"
	./$(TEST_TARGET)

clean:
	rm -f $(OBJS) $(LIB_OBJS) $(TARGET) $(LIB_NAME) $(TEST_TARGET) test_spravochnik.o

.PHONY: all clean test