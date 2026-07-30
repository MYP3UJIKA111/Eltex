CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -g -fPIC
LDFLAGS = -lm -ldl

TARGET = calculator
MAIN_SRC = main.c
MAIN_OBJ = $(MAIN_SRC:.c=.o)
DEPS = calculator.h

PLUGIN_DIR = plugins
PLUGIN_SRC_DIR = src
PLUGINS_SRC = $(wildcard $(PLUGIN_SRC_DIR)/*.c)
PLUGINS_SO = $(patsubst $(PLUGIN_SRC_DIR)/%.c,$(PLUGIN_DIR)/lib%.so,$(PLUGINS_SRC))

all: $(TARGET) plugins

$(TARGET): $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

plugins: $(PLUGINS_SO)

$(PLUGIN_DIR)/lib%.so: $(PLUGIN_SRC_DIR)/%.c
	@mkdir -p $(PLUGIN_DIR)
	$(CC) $(CFLAGS) -shared -o $@ $<

clean:
	rm -f $(MAIN_OBJ) $(TARGET)
	rm -rf $(PLUGIN_DIR)

rebuild: clean all

run: $(TARGET) plugins
	./$(TARGET)

.PHONY: all clean rebuild run plugins