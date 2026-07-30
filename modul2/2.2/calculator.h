#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double add(double a, double b);
double subtract(double a, double b);
double multiply(double a, double b);
double divide(double a, double b);
double power(double a, double b);
double square_root(double a);
double sine(double a);
double cosine(double a);
double tangent(double a);

void clearInputBuffer(void);
void showMenu(void);

#endif