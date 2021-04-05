#include <string.h>
#include <math.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#include "file.h"
#include "shared_memory.h"
#include "common.h"
#define CLIENT_SEPARATOR " "

// CLIENT-SIDE FUNCTIONS

enum Client_Status
{
    ERROR = 1,
    QUIT = 2
};

typedef struct server_range
{
    unsigned int start;
    unsigned int end;
} server_range_t;

typedef struct reader_args
{
    unsigned int start;
    unsigned int end;
    unsigned int server_number;
    unsigned int mem_size;
    unsigned int server_base_port;
    char server_base_ip[20];
} reader_args_t;

typedef struct writer_args
{
    char *string;

    unsigned int start;
    unsigned int end;
    unsigned int server_number;
    unsigned int mem_size;
    unsigned int server_base_port;
    char server_base_ip[20];
} writer_args_t;

server_range_t verify_which_servers(int start, int end, unsigned int memory_size);

int user_read_request(char request[REQ_SIZE], configs_t configs);
int user_write_request(char request[REQ_SIZE], configs_t configs);

void *writer(void *arg);
void *reader(void *arg);

int user_read_request(char request[REQ_SIZE], configs_t configs)
{
    // Extract the start and end positions
    strtok(request, CLIENT_SEPARATOR); // r
    int start = atoi(strtok(NULL, CLIENT_SEPARATOR));
    int size = atoi(strtok(NULL, CLIENT_SEPARATOR));
    int end = start + size - 1;

    unsigned int mem_size = configs.memory_size;

    // Verify which servers we need to connect
    server_range_t range = verify_which_servers(start, end, mem_size);

    unsigned int number_of_readers = range.end - range.start + 1;

    if (range.start > configs.number_of_servers - 1)
    {
        printf("[ERROR] You are trying to pick up from a position (%d) that does not exist in memory. Our last position is %d\n", start, configs.memory_size * configs.number_of_servers - 1);
        return ERROR; // Error
    }

    if (range.end > configs.number_of_servers - 1)
    {
        printf("[ERROR] You're trying to get more characters (%d) than memory have (%d)\n", size, configs.memory_size * configs.number_of_servers);
        return ERROR; // Error
    }

    pthread_t readers[number_of_readers];
    reader_args_t args[number_of_readers];

    unsigned int start_index = start % mem_size;
    unsigned int end_index = end % mem_size;

    for (int i = 0; i < number_of_readers; i++)
    {
        args[i].server_number = range.start++;
        args[i].start = 0;
        args[i].end = mem_size - 1;
        args[i].server_base_port = configs.server_base_port;
        args[i].mem_size = mem_size;
        strcpy(args[i].server_base_ip, configs.server_base_ip);

        if (args[i].server_number == range.start)
        {
            args[i].start = start_index;
        }

        if (args[i].server_number == range.end)
        {
            args[i].end = end_index;
        }

        pthread_create(&(readers[i]), NULL, reader, &(args[i]));
    }

    char *result = malloc(sizeof(char) * number_of_readers * mem_size);
    memset(result, 0, strlen(result));

    for (int i = 0; i < number_of_readers; i++)
    {
        char *temp;
        pthread_join(readers[i], (void **)&temp);
        strcat(result, temp);
    }

    printf("MEMORY[%d:%d]=%s\n", start, end, result);
    
    free(result);

    return 0;
}

server_range_t verify_which_servers(int start, int end, unsigned int memory_size)
{
    server_range_t range;
    range.start = floor(start / memory_size);
    range.end = floor(end / memory_size);
    return range;
}

int user_write_request(char request[REQ_SIZE], configs_t configs)
{
    // Extract the start, the string and the size
    strtok(request, CLIENT_SEPARATOR); // w
    int start = atoi(strtok(NULL, CLIENT_SEPARATOR));
    char *string = strtok(NULL, CLIENT_SEPARATOR);
    int size = atoi(strtok(NULL, CLIENT_SEPARATOR));
    int end = start + size - 1;

    if (size <= 0)
    {
        printf("[ERROR] You can't write with size less or equal than 0!\n");
        return ERROR; // Error
    }

    unsigned int mem_size = configs.memory_size;

    // Verify which servers we need to connect
    server_range_t range = verify_which_servers(start, end, mem_size);

    if (range.start > configs.number_of_servers - 1)
    {
        printf("[ERROR] You are trying to write up from a position (%d) that does not exist in memory. Our last position is %d!\n", start, configs.memory_size * configs.number_of_servers - 1);
        return ERROR; // Error
    }

    if (range.end > configs.number_of_servers - 1)
    {
        printf("[ERROR] You're trying to write more characters (%d) than memory have (%d)!\n", size, configs.memory_size * configs.number_of_servers);
        return ERROR; // Error
    }

    unsigned int number_of_writers = range.end - range.start + 1;

    pthread_t writers[number_of_writers];
    writer_args_t args[number_of_writers];

    for (int i = 0; i < number_of_writers; i++)
    {
        args[i].string = malloc(sizeof(char) * mem_size);
    }

    // Separate the strings for the threads
    char c;
    int mem_count = start % mem_size; // Control the actual index on memory
    int j = 0;             // Control the index of args[i].string
    int thread = 0;        // Control the thread that will receive the char on args
    for (int i = 0; i < size; i++)
    {
        c = string[i];
        if (mem_count == mem_size)
        {
            thread++;
            mem_count = 0;
            j = 0;
        }
        args[thread].string[j] = c;
        j++;
        mem_count++;
    }

    unsigned int start_index = start % mem_size;
    unsigned int end_index = end % mem_size;

    int server_number = range.start;
    for (int i = 0; i < number_of_writers; i++)
    {
        args[i].server_number = server_number++;
        args[i].start = 0;
        args[i].end = mem_size - 1;
        args[i].server_base_port = configs.server_base_port;
        args[i].mem_size = mem_size;
        strcpy(args[i].server_base_ip, configs.server_base_ip);

        if (args[i].server_number == range.start)
        {
            args[i].start = start_index;
        }

        if (args[i].server_number == range.end)
        {
            args[i].end = end_index;
        }

        pthread_create(&(writers[i]), NULL, writer, &(args[i]));
    }

    for (int  i = 0; i < number_of_writers; i++) {
        pthread_join(writers[i], NULL);
        free(args[i].string);
    }

    return 0;
}

void *writer(void *arg)
{
    writer_args_t * args = ( writer_args_t *)arg;

    // Configuring to connect with the server
    int sockfd, len, result;
    struct sockaddr_in address;
    unsigned int mem_size = args->mem_size;
    char answer[mem_size], write_request[mem_size];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(args->server_base_ip);
    address.sin_port = args->server_base_port + args->server_number;
    len = sizeof(address);

    result = connect(sockfd, (struct sockaddr *)&address, len);

    if (result == -1)
    {
        perror("Was not possible to read the memory :(\n");
        exit(1);
    }

    sprintf(write_request, "w#%d#%d#%s", args->start, args->end, args->string);
    write(sockfd, &write_request, REQ_SIZE);
    // read(sockfd, &answer, mem_size); // Precisa?
}

void *reader(void *arg)
{
    reader_args_t *args = (reader_args_t *)arg;

    // Configuring to connect with the server
    int sockfd, len, result;
    struct sockaddr_in address;
    unsigned int mem_size = args->mem_size;
    char answer[mem_size], read_request[mem_size];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(args->server_base_ip);
    address.sin_port = args->server_base_port + args->server_number;
    len = sizeof(address);

    result = connect(sockfd, (struct sockaddr *)&address, len);

    if (result == -1)
    {
        perror("Was not possible to read the memory :(\n");
        return 0;
    }

    sprintf(read_request, "r#%d#%d", args->start, args->end);
    write(sockfd, &read_request, REQ_SIZE);
    read(sockfd, &answer, mem_size);
    pthread_exit((void *)answer);
}
