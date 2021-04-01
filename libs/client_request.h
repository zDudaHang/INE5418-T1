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

#define REQ_SIZE 100
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
    char * string;

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

void *reader(void *arg);

char *make_read_request(int start, int end);

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

    if (range.start > configs.number_of_servers - 1) {
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
        args[i].server_number = i;
        args[i].start = 0;
        args[i].end = mem_size - 1;
        args[i].server_base_port = configs.server_base_port;
        args[i].mem_size = mem_size;
        strcpy(args[i].server_base_ip, configs.server_base_ip);

        if (i == range.start)
        {
            args[i].start = start_index;
        }

        if (i == range.end)
        {
            args[i].end = end_index;
        }

        pthread_create(&(readers[i]), NULL, reader, &(args[i]));
    }

    char * result = malloc(sizeof(char) * number_of_readers * mem_size);

    for (int i = 0; i < number_of_readers; i++)
    {
        char * temp;
        pthread_join(readers[i], (void **)&temp);
        strcat(result, temp);
    }

    printf("MEMORY[%d:%d]=%s\n", start, end, result);

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
    char * string = strtok(NULL, CLIENT_SEPARATOR);
    int size = atoi(strtok(NULL, CLIENT_SEPARATOR));
    int end = start + size - 1;

    printf("START=%d, STRING=%s, SIZE=%d\n", start, string, size);

    if (size <= 0) {
        printf("[ERROR] You can't write with size less or equal than 0!\n");
        return ERROR; // Error
    }

    unsigned int mem_size = configs.memory_size;

    // TODO Check if the user is not passing a size less or greater than the real string!

    // Verify which servers we need to connect
    server_range_t range = verify_which_servers(start, end, mem_size);

    printf("Range[start=%d, end=%d]\n", range.start, range.end);

    if (range.start > configs.number_of_servers - 1) {
        printf("[ERROR] You are trying to write up from a position (%d) that does not exist in memory. Our last position is %d\n", start, configs.memory_size * configs.number_of_servers - 1);
        return ERROR; // Error
    }

    if (range.end > configs.number_of_servers - 1)
    {
        printf("[ERROR] You're trying to write more characters (%d) than memory have (%d)\n", size, configs.memory_size * configs.number_of_servers);
        return ERROR; // Error
    }

    unsigned int number_of_writers = range.end - range.start + 1;

    writer_args_t args[number_of_writers];

    printf("Preparing the strings...\n");

    char c;
    char * str;
    int mem_count = start;
    int thread = 0;
    for (int i = 0; i < size; i++) {
        c = string[i];
        printf("c=%c\n", c);
        printf("thread=%d\n", thread);
        if (mem_count == mem_size) {
            thread++;
            mem_count = 0;
            strcat(str, "#");
        }
        strncat(str, &c, 1);
    } 

    printf("STR=%s\n", str);

    printf("Done!\n");

    for (int i = 0; i < number_of_writers; i++) {
        printf("args[%d]=%s\n", i, args[i].string);
    }

    return 0;
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
        // TODO Change the message because the user not should know that's a shared memory
        perror("ERROR WHEN CONNECTING!\n");
        exit(1);
    }

    sprintf(read_request, "r#%d#%d", args->start, args->end);
    write(sockfd, &read_request, REQ_SIZE);
    read(sockfd, &answer, mem_size);
    pthread_exit((void *)answer);
}
