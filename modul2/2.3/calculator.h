#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

#define MAX_COMMANDS 20
#define MAX_CMD_NAME 30

typedef double (*variadic_op_t)(int, ...);

typedef struct {
    char name[MAX_CMD_NAME];
    char symbol[MAX_CMD_NAME];
    variadic_op_t func;
    int is_available;
} Command;

double sum(int num_args, ...);
double multiply_variadic(int num_args, ...);
double max_value(int num_args, ...);
double min_value(int num_args, ...);
double average(int num_args, ...);

void initCommands(Command commands[], int* count);
void addCommand(Command commands[], int* count, const char* name, 
                const char* symbol, variadic_op_t func);
void showDynamicMenu(Command commands[], int count);
void executeCommand(Command commands[], int count, int choice);
void clearInputBuffer(void);
double getNumber(const char* prompt);
int getArgumentCount(void);

#endif