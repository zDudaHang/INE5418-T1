#ifndef INE5418_T1_FILE_H
#define INE5418_T1_FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_PATH "./config/config.txt"
#define SIZE 200

typedef struct configs
{
    unsigned int memory_size;
    char server_base_ip[20];
    unsigned int server_base_port;
    unsigned int server_range_index;
    unsigned int number_of_servers;
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
        "number_of_servers=%d",
        &configs.memory_size,
        configs.server_base_ip,
        &configs.server_base_port,
        &configs.server_range_index,
        &configs.number_of_servers);

    fclose(f);

    return configs;
}

#endif