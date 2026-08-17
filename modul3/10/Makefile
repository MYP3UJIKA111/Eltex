CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGETS = taxi_manager taxi_driver

all: $(TARGETS)

taxi_manager: taxi_manager.c
	$(CC) $(CFLAGS) -o $@ $^

taxi_driver: taxi_driver.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)

run: taxi_manager
	./taxi_manager

.PHONY: all clean run