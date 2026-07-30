#include "priority_queue.h"

QueueItem* createItem(int priority, const char* message) {
    if (priority < MIN_PRIORITY || priority > MAX_PRIORITY) {
        fprintf(stderr, "Ошибка: Приоритет должен быть от %d до %d\n", MIN_PRIORITY, MAX_PRIORITY);
        return NULL;
    }
    
    QueueItem* newItem = (QueueItem*)malloc(sizeof(QueueItem));
    if (newItem == NULL) {
        fprintf(stderr, "Ошибка выделения памяти!\n");
        return NULL;
    }
    
    newItem->priority = priority;
    strncpy(newItem->message, message, MAX_MSG_LEN - 1);
    newItem->message[MAX_MSG_LEN - 1] = '\0';
    newItem->next = NULL;
    newItem->prev = NULL;
    
    return newItem;
}

QueueItem* enqueue(QueueItem* head, int priority, const char* message) {
    QueueItem* newItem = createItem(priority, message);
    if (newItem == NULL) return head;
    
    if (head == NULL) {
        newItem->next = newItem;
        newItem->prev = newItem;
        return newItem;
    }
    
    if (newItem->priority >= head->priority) {
        newItem->next = head;
        newItem->prev = head->prev;
        head->prev->next = newItem;
        head->prev = newItem;
        return newItem;
    }
    
    QueueItem* tmp = head;
    do {
        if (newItem->priority >= tmp->priority) {
            newItem->next = tmp;
            newItem->prev = tmp->prev;
            tmp->prev->next = newItem;
            tmp->prev = newItem;
            return head;
        }
        tmp = tmp->next;
    } while (tmp != head);
    
    newItem->next = head;
    newItem->prev = head->prev;
    head->prev->next = newItem;
    head->prev = newItem;
    
    return head;
}

QueueItem* dequeueFirst(QueueItem** head) {
    if (*head == NULL) return NULL;
    
    QueueItem* item = *head;
    
    if (item->next == item && item->prev == item) {
        *head = NULL;
    } else {
        item->next->prev = item->prev;
        item->prev->next = item->next;
        *head = item->next;
    }
    
    return item;
}

QueueItem* dequeueByPriority(QueueItem** head, int priority) {
    if (*head == NULL) return NULL;
    
    QueueItem* tmp = *head;
    
    do {
        if (tmp->priority == priority) {
            if (tmp->next == tmp && tmp->prev == tmp) {
                *head = NULL;
            } else {
                tmp->next->prev = tmp->prev;
                tmp->prev->next = tmp->next;
                if (tmp == *head) {
                    *head = tmp->next;
                }
            }
            return tmp;
        }
        tmp = tmp->next;
    } while (tmp != *head);
    
    return NULL;
}

QueueItem* dequeueByMinPriority(QueueItem** head, int minPriority) {
    if (*head == NULL) return NULL;
    
    QueueItem* tmp = (*head)->prev;
    QueueItem* startNode = tmp;
    
    do {
        if (tmp->priority >= minPriority) {
            if (tmp->next == tmp && tmp->prev == tmp) {
                *head = NULL;
            } else {
                tmp->next->prev = tmp->prev;
                tmp->prev->next = tmp->next;
                if (tmp == *head) {
                    *head = tmp->next;
                }
            }
            return tmp;
        }
        tmp = tmp->prev;
    } while (tmp != startNode);
    
    return NULL;
}

void printQueue(QueueItem* head) {
    if (head == NULL) {
        printf("Очередь пуста.\n");
        return;
    }
    
    QueueItem* tmp = head;
    int count = 1;
    
    printf("=== Очередь с приоритетом (%d элементов) ===\n", getSize(head));
    do {
        printf("[%d] Приоритет: %3d | Сообщение: %s\n", count, tmp->priority, tmp->message);
        tmp = tmp->next;
        count++;
    } while (tmp != head);
    printf("=============================================\n");
}

void freeQueue(QueueItem* head) {
    if (head == NULL) return;
    
    QueueItem* tmp = head;
    QueueItem* itemToRemove;
    
    do {
        itemToRemove = tmp;
        tmp = tmp->next;
        free(itemToRemove);
    } while (tmp != head);
}

int isEmpty(QueueItem* head) {
    return (head == NULL);
}

int getSize(QueueItem* head) {
    if (head == NULL) return 0;
    
    QueueItem* tmp = head;
    int count = 0;
    
    do {
        count++;
        tmp = tmp->next;
    } while (tmp != head);
    
    return count;
}