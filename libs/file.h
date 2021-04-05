#ifndef INE5418_T1_FILE_H
#define INE5418_T1_FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FILE_PATH "./config/config.txt"
#define SIZE 200
#define LOG_PATH "./logs/log"

typedef struct configs
{
    unsigned int memory_size;
    unsigned int server_base_port;
    unsigned int server_range_index;
    unsigned int number_of_servers;
    unsigned int log_time;
    char server_base_ip[20];

} configs_t;

configs_t read_configs()
{
    FILE *f;
    configs_t configs;

    f = fopen(FILE_PATH, "r");

    if (f == NULL)
    {
        perror("Was not possible to open the file");
        exit(1);
    }

    fscanf(
        f,
        "memory_size=%d\n"
        "server_base_ip=%s\n"
        "server_base_port=%d\n"
        "server_range_index=%d\n"
        "number_of_servers=%d\n"
        "log_time=%d",

        &configs.memory_size,
        configs.server_base_ip,
        &configs.server_base_port,
        &configs.server_range_index,
        &configs.number_of_servers,
        &configs.log_time
    
    );

    fclose(f);

    return configs;
}

void write_log(char * data) {
    FILE * f;
    time_t timestamp = time(NULL);
    char filename[SIZE];
    sprintf(filename, "%s_%ld.txt", LOG_PATH, timestamp);
    f = fopen(filename, "w");

    if (f == NULL) {
        perror("Unable to create log file :(\n");
        exit(1);
    }

    fputs(data, f);

    fclose(f);
}

#endif