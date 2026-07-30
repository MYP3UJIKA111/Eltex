#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

#define BUFFER_SIZE 256
#define MAX_PATH 1024

typedef struct {
    char symbolic[12];      // Символьное представление
    int numeric;            // Числовое представление
    unsigned short binary;  // Битовое представление (12 бит)
    char filepath[MAX_PATH];// Путь к файлу (или "<Ручной ввод>")
    int valid;              // Флаг: загружены ли данные
    mode_t st_mode;         // Полный режим из stat
    uid_t st_uid;           // UID владельца
    gid_t st_gid;           // GID группы
    char owner[256];        // Имя владельца
    char group[256];        // Имя группы
    off_t st_size;          // Размер файла
    time_t st_mtime_val;    // Время модификации
} FilePermissions;

// Преобразования форматов
void parseNumericToSymbolic(int numeric, char* symbolic);
int parseSymbolicToNumeric(const char* symbolic);
int parseNumericInput(const char* str);
void printBinary(unsigned short value, char* buffer, int size);

// Работа с файлами
int getPermissionsFromFile(const char* filename, FilePermissions* perm);
void displayPermissions(const FilePermissions* perm);
void displayComparison(const char* filename);
void displayFileInfo(const FilePermissions* perm);

// Изменение прав в буфере
int modifyPermissionsInBuffer(FilePermissions* perm, const char* command);

// Вспомогательные функции
void clearInputBuffer(void);
int isValidSymbolic(const char* str);
int isValidNumeric(const char* str);
void printHelp(void);
void savePermissionsToBuffer(FilePermissions* perm, int mode, const char* filename);

#endif