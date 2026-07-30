#ifndef SPRAVOCHNIK_H
#define SPRAVOCHNIK_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <wctype.h>
#include <wchar.h>

#define MAX_STR 60
#define MAX_PHONES 3
#define MAX_EMAILS 2
#define MAX_SOCIALS 2
#define MAX_MESSENGERS 2

struct abonent {
    int id;
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

struct TreeNode {
    struct abonent data;
    int height;
    struct TreeNode* left;
    struct TreeNode* right;
};

extern struct TreeNode* root;
extern int next_id;

void clearInputBuffer(void);
int strcmpIgnoreCase(const char* s1, const char* s2);
int isValidName(const char* name);
int isValidPhone(const char* phone);
int contains_ignore_case(const char* haystack, const char* needle);

struct TreeNode* createNode(struct abonent data);
int getHeight(struct TreeNode* node);
int getBalance(struct TreeNode* node);
struct TreeNode* rightRotate(struct TreeNode* y);
struct TreeNode* leftRotate(struct TreeNode* x);
struct TreeNode* insert(struct TreeNode* node, struct abonent data);
struct TreeNode* minValueNode(struct TreeNode* node);
struct TreeNode* deleteNode(struct TreeNode* root, const char* surname, const char* firstname);
struct TreeNode* searchExact(struct TreeNode* root, const char* surname, const char* firstname);
void searchByName(struct TreeNode* root, const char* name);
void printInOrder(struct TreeNode* root);
void printTreeVisual(struct TreeNode* root, int space);
void freeTree(struct TreeNode* root);

void addAbonent(void);
void deleteAbonent(void);
void editAbonent(void);
void searchMenu(void);
void displayAll(void);
void displayAsTree(void);
void showMenu(void);

#endif