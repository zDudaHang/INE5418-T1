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

#define REQ_SIZE 50
#define CLIENT_SEPARATOR " "

// CLIENT-SIDE FUNCTIONS 

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

server_range_t verify_which_servers(int start, int end, unsigned int memory_size);

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

    if (range.start > configs.number_of_servers || range.end > configs.number_of_servers)
    {
        return 1; // Error
    }

    pthread_t readers[number_of_readers];
    reader_args_t args[number_of_readers];

    unsigned int start_index = start % mem_size;
    unsigned int end_index = end % mem_size;

    if (range.start == range.end)
    {
        args[0].start = start_index;
        args[0].end = end_index;
        args[0].server_number = range.start;
        args[0].server_base_port = configs.server_base_port;
        args[0].mem_size = mem_size;
        strcpy(args[0].server_base_ip, configs.server_base_ip);
        reader(&(args[0]));
        return 0;
    }

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

    for (int i = 0; i < number_of_readers; i++)
    {
        pthread_join(readers[i], NULL);
    }

    return 0;
}

server_range_t verify_which_servers(int start, int end, unsigned int memory_size)
{
    server_range_t range;
    range.start = floor(start / memory_size);
    range.end = floor(end / memory_size);
    return range;
}

int user_write_request(char input[REQ_SIZE], configs_t configs)
{
}

void *reader(void *arg)
{
    reader_args_t *args = (reader_args_t *)arg;
    printf("I'm a reader! I have to read S%d[%d,%d]\n", args->server_number, args->start, args->end);

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
        perror("ERROR WHEN CONNECTING!\n");
        exit(1);
    }

    sprintf(read_request, "r#%d#%d", args->start, args->end);

    printf("Trying to write %s on socket...\n", read_request);
    write(sockfd, &read_request, REQ_SIZE);
    read(sockfd, &answer, mem_size);
    printf("%s\n", answer);
    printf("Done!\n");
    return 0;
}
