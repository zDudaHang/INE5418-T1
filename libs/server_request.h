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
#define SERVER_SEPARATOR "#"
#define REQ_SIZE 50

// SERVER-SIDE FUNCTIONS 

typedef struct fragmented_client_read_request
{
    unsigned int start;
    unsigned int end;
} fragmented_client_read_request_t;

typedef struct handler_args {
    int client_sockfd;
    shared_memory_element_t * memory;
    configs_t configs;
} handler_args_t;

void verify_client_request(char * result, char type, char *request, configs_t configs, shared_memory_element_t * memory);
void client_read_request(char * result, char * request, configs_t configs, shared_memory_element_t * memory);
fragmented_client_read_request_t split_read_request(char * request);
void * handler(void * arg);

void verify_client_request(char * result, char type, char *request, configs_t configs, shared_memory_element_t * memory)
{
    switch (type)
    {
    case 'r':
        client_read_request(result, request, configs, memory);
        break;
    case 'w':
        break;
    default:
        break;
    }
}

void client_read_request(char * result, char * request, configs_t configs, shared_memory_element_t * memory) {
    fragmented_client_read_request_t frag_req = split_read_request(request);
    read_memory(memory, result, frag_req.start, frag_req.end);
}

fragmented_client_read_request_t split_read_request(char * request) {
    fragmented_client_read_request_t frag_req;
    strtok(request, SERVER_SEPARATOR); // r
    frag_req.start = atoi(strtok(NULL, SERVER_SEPARATOR));
    frag_req.end = atoi(strtok(NULL, SERVER_SEPARATOR));
    return frag_req;
}

void * handler(void * arg) {
    handler_args_t * args = (handler_args_t *) arg;
    char request[REQ_SIZE], result[REQ_SIZE] = "";
    read(args->client_sockfd, &request, REQ_SIZE);
    verify_client_request(result, request[0], request, args->configs, args->memory);
    write(args->client_sockfd, &result, sizeof(result));
    close(args->client_sockfd);
}