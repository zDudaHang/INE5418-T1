#include <string.h>
#include <math.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdlib.h>

#include "file.h"
#include "shared_memory.h"
#include "common.h"
#define SERVER_SEPARATOR "#"

// SERVER-SIDE FUNCTIONS 

typedef struct fragmented_client_read_request
{
    unsigned int start;
    unsigned int end;
} fragmented_client_read_request_t;

typedef struct fragmented_client_write_request
{
    unsigned int start;
    unsigned int end;
    char * string;
} fragmented_client_write_request_t;

typedef struct handler_args {
    int client_sockfd;
    shared_memory_element_t * memory;
    configs_t configs;
} handler_args_t;

void verify_client_request(char * result, char type, char *request, configs_t configs, shared_memory_element_t * memory);
void client_read_request(char * result, char * request, configs_t configs, shared_memory_element_t * memory);
void client_write_request(char * result, char * request, configs_t configs, shared_memory_element_t * memory);

fragmented_client_read_request_t split_read_request(char * request);
fragmented_client_write_request_t split_write_request(char * request);

void * handler(void * arg);

void verify_client_request(char * result, char type, char *request, configs_t configs, shared_memory_element_t * memory)
{
    switch (type)
    {
    case 'r':
        client_read_request(result, request, configs, memory);
        break;
    case 'w':
        client_write_request(result, request, configs, memory);
        break;
    default:
        break;
    }
}

void client_read_request(char * result, char * request, configs_t configs, shared_memory_element_t * memory) {
    fragmented_client_read_request_t frag = split_read_request(request);
    read_memory(memory, result, frag.start, frag.end);
}

void client_write_request(char * result, char * request, configs_t configs, shared_memory_element_t * memory) {
    fragmented_client_write_request_t frag = split_write_request(request);
    
    int i = 0;
    char c = frag.string[i];
    while(frag.start <= frag.end) {
        write_memory(memory, c, frag.start++);
        c = frag.string[++i];
    }
}

fragmented_client_write_request_t split_write_request(char * request) {
    fragmented_client_write_request_t frag;
    strtok(request, SERVER_SEPARATOR); // r
    frag.start = atoi(strtok(NULL, SERVER_SEPARATOR));
    frag.end = atoi(strtok(NULL, SERVER_SEPARATOR));
    frag.string = strtok(NULL, SERVER_SEPARATOR);
    return frag;
}

fragmented_client_read_request_t split_read_request(char * request) {
    fragmented_client_read_request_t frag;
    strtok(request, SERVER_SEPARATOR); // r
    frag.start = atoi(strtok(NULL, SERVER_SEPARATOR));
    frag.end = atoi(strtok(NULL, SERVER_SEPARATOR));
    return frag;
}

void * handler(void * arg) {
    handler_args_t * args = (handler_args_t *) arg;
    char request[REQ_SIZE], result[REQ_SIZE] = "";
    read(args->client_sockfd, &request, REQ_SIZE);
    verify_client_request(result, request[0], request, args->configs, args->memory);
    write(args->client_sockfd, &result, sizeof(result));
    close(args->client_sockfd);
}