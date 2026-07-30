#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <dirent.h>

#define MAX_COMMANDS 50
#define MAX_CMD_NAME 50
#define MAX_PATH_LEN 512
#define PLUGINS_DIR "plugins"

typedef double (*variadic_op_t)(int, ...);
typedef const char* (*string_getter_t)(void);

typedef struct {
    char name[MAX_CMD_NAME];
    char symbol[MAX_CMD_NAME];
    char lib_path[MAX_PATH_LEN];
    variadic_op_t func;
    void* handle;
    int is_available;
} Command;

void clearInputBuffer(void);
double getNumber(const char* prompt);
int getArgumentCount(void);
void showDynamicMenu(Command commands[], int count);
void executeCommand(Command commands[], int count, int choice);
int loadPlugins(Command commands[], int* count);
void unloadPlugins(Command commands[], int count);

#endif