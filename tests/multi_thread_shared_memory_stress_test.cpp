#include <stdio.h>
#include <assert.h>
#include "../shared_memory.h"

// MULTI-THREAD SHARED MEMORY LIB TEST
#define NUM_THREADS 100
typedef struct {
    int read_start, read_end, write_index;
    char value;
    shared_memory_element_t * memory;
} thread_arg_t ;

void * writer(void *arg) {
    thread_arg_t * args = (thread_arg_t *) arg;
    int write_index = args->write_index;
    char value = args->value;
    shared_memory_element_t * memory = (shared_memory_element_t *) args->memory;
    write(memory, value, write_index);
    return 0;
}

void * reader(void *arg) {
    thread_arg_t * args = (thread_arg_t *) arg;
    int read_start = args->read_start;
    int read_end = args->read_end;
    shared_memory_element_t * memory = (shared_memory_element_t *) args->memory;
    char result[read_end - read_start];
    read(memory, result, read_start, read_end);
    return 0;
}

pthread_t threads[NUM_THREADS];

int main() {
    shared_memory_element_t memory[NUM_THREADS];

    thread_arg_t args[NUM_THREADS];

    init_memory(memory, NUM_THREADS);

    for(int i = 0; i < NUM_THREADS; i++) {
        args[i].write_index = i % 2 == 0 ? i : i - 1;
        args[i].read_start = i > 2 ? i - 2 : i;
        args[i].read_end = i < 99 ? i + 1 : i;
        args[i].value = (char ) 'a' + (i % 20);
        args[i].memory = memory;
        pthread_create(&(threads[i]), NULL, i % 2 == 0 ? writer : reader, &(args[i]));
    }

    for(int j = 0; j < NUM_THREADS; j++) {
        pthread_join(threads[j], NULL);
    }

    return 0;
}

