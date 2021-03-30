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

char* client_read_request(char * request, configs_t configs, shared_memory_element_t * memory);
fragmented_client_read_request_t split_read_request(char * request);

char* client_read_request(char * request, configs_t configs, shared_memory_element_t * memory) {
    char result[configs.memory_size];
    fragmented_client_read_request_t frag_req = split_read_request(request);
    read_memory(memory, result, frag_req.start, frag_req.end);
    printf("Readed: %s\n", result);
    return result;
}

fragmented_client_read_request_t split_read_request(char * request) {
    fragmented_client_read_request_t frag_req;

    strtok(request, SERVER_SEPARATOR); // r
    frag_req.start = atoi(strtok(NULL, SERVER_SEPARATOR));
    frag_req.end = atoi(strtok(NULL, SERVER_SEPARATOR));

    printf("Frag_req[start=%d, end=%d]\n", frag_req.start, frag_req.end);

    return frag_req;
}