#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_MSG_LEN 100
#define MAX_PRIORITY 255
#define MIN_PRIORITY 0

typedef struct QueueItem {
    int priority;
    char message[MAX_MSG_LEN];
    struct QueueItem* next;
    struct QueueItem* prev;
} QueueItem;

QueueItem* createItem(int priority, const char* message);
QueueItem* enqueue(QueueItem* head, int priority, const char* message);
QueueItem* dequeueFirst(QueueItem** head);
QueueItem* dequeueByPriority(QueueItem** head, int priority);
QueueItem* dequeueByMinPriority(QueueItem** head, int minPriority);
void printQueue(QueueItem* head);
void freeQueue(QueueItem* head);
int isEmpty(QueueItem* head);
int getSize(QueueItem* head);

#endif