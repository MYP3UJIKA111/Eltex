#ifndef SPRAVOCHNIK_H
#define SPRAVOCHNIK_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <wctype.h>
#include <wchar.h>

#define MAX_ABONENTS 100
#define MAX_STR 60
#define MAX_PHONES 3
#define MAX_EMAILS 2
#define MAX_SOCIALS 2
#define MAX_MESSENGERS 2

struct abonent {
    char first_name[MAX_STR];
    char second_name[MAX_STR];
    char patronymic[MAX_STR];
    char workplace[MAX_STR];
    char position[MAX_STR];
    char phones[MAX_PHONES][MAX_STR];
    char emails[MAX_EMAILS][MAX_STR];
    char socials[MAX_SOCIALS][MAX_STR];
    char messengers[MAX_MESSENGERS][MAX_STR];
};

extern struct abonent database[MAX_ABONENTS];
extern int count;

void removeNewline(char* str);
void clearEntry(struct abonent* ab);
void clearInputBuffer(void);
int strcmpIgnoreCase(const char* s1, const char* s2);
int isOnlySpaces(const char* str);
int isValidName(const char* name);
int isValidPhone(const char* phone);
int contains_ignore_case(const char* haystack, const char* needle);

void addAbonent(void);
void deleteAbonent(void);
void editAbonent(void);
void searchByName(void);
void displayAll(void);
void showMenu(void);

int addAbonentDirect(const struct abonent* new_ab);
int deleteAbonentByIndex(int index);
int findAbonentByName(const char* name);

#endif