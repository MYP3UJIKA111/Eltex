CC = gcc
CFLAGS = -D_GNU_SOURCE -Wall -Werror -Wextra -pedantic -std=c11 -g
TARGET = spravochnik
LIB_NAME = libspravochnik.so

SRCS = main.c
LIB_SRCS = spravochnik.c
OBJS = $(SRCS:.c=.o)
LIB_OBJS = $(LIB_SRCS:.c=.o)
DEPS = spravochnik.h

TEST_TARGET = run_tests

all: $(TARGET)

# Компиляция объектных файлов с флагом -fPIC (для динамической библиотеки)
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

# Создание динамической библиотеки
$(LIB_NAME): $(LIB_OBJS)
	@echo "Создание динамической библиотеки $(LIB_NAME)..."
	$(CC) -shared -o $(LIB_NAME) $(LIB_OBJS)
	@echo "Динамическая библиотека успешно создана!"

# Компоновка исполняемого файла с динамической библиотекой
$(TARGET): $(OBJS) $(LIB_NAME)
	@echo "Компоновка с динамической библиотекой $(LIB_NAME)..."
	$(CC) $(CFLAGS) -o $@ $(OBJS) -L. -lspravochnik -Wl,-rpath,'$$ORIGIN'

# Компиляция тестов
test: $(LIB_NAME) test_spravochnik.c
	@echo "Компиляция тестов..."
	$(CC) $(CFLAGS) -o $(TEST_TARGET) test_spravochnik.c -L. -lspravochnik -Wl,-rpath,'$$ORIGIN'
	@echo "Запуск тестов...\n"
	./$(TEST_TARGET)

clean:
	rm -f $(OBJS) $(LIB_OBJS) $(TARGET) $(LIB_NAME) $(TEST_TARGET) test_spravochnik.o

.PHONY: all clean test