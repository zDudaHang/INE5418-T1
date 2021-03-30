#ifndef INE5418_T1_SHARED_MEMORY_H
#define INE5418_T1_SHARED_MEMORY_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct shared_memory_element {
    char value;
    pthread_rwlock_t lock;
} shared_memory_element_t;

int init_memory(shared_memory_element_t * memory, int size) {
    for (int i = 0; i < size; i++) {
        memory[i].value = 'a';
        int result = pthread_rwlock_init(&(memory[i].lock), NULL);
        if (result != 0) {
            perror("Can't init the read write lock");
            exit(1);
        }
    }
    return 0;
}

char * read_memory(shared_memory_element_t * memory, char * result, int start, int end) {
    int j = 0;
    for (int i = start; i <= end; i++) {
        pthread_rwlock_rdlock(&(memory[i].lock));
        result[j] = memory[i].value;
        pthread_rwlock_unlock(&(memory[i]).lock);
        j++;
    }
    return result;
}

void write_memory(shared_memory_element_t * memory, char value, int index) {
    pthread_rwlock_wrlock(&(memory[index].lock));
    memory[index].value = value;
    pthread_rwlock_unlock(&(memory[index].lock));
}


#endif //INE5418_T1_SHARED_MEMORY_H
