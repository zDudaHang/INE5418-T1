#include "client_request.h"

int isLoggerLeader(int id, int number_of_servers);
void *logger(void *arg);

typedef struct logger_args
{
    int server_sockfd;
    configs_t configs;
    shared_memory_element_t *memory;
} logger_args_t;

int isLoggerLeader(int id, int number_of_servers)
{
    return id == number_of_servers - 1;
}

void *logger(void *arg)
{
    logger_args_t *args = (logger_args_t *)arg;
    configs_t configs = args->configs;
    
    while(1) {
        sleep(configs.log_time); // sleep a while

        // Leader side
        const int num_servers = configs.number_of_servers - 1;
        const int mem_size = configs.memory_size;

        if (isLoggerLeader(configs.server_range_index, configs.number_of_servers))
        {
            if (num_servers >= 1)
            {
                pthread_t readers[num_servers];
                reader_args_t r_args[num_servers];

                for (int i = 0; i < num_servers; i++)
                {
                    r_args[i].server_number = i;
                    r_args[i].start = 0;
                    r_args[i].end = mem_size - 1;
                    r_args[i].server_base_port = configs.server_base_port;
                    r_args[i].mem_size = mem_size;
                    strcpy(r_args[i].server_base_ip, configs.server_base_ip);

                    pthread_create(&(readers[i]), NULL, reader, &(r_args[i]));
                }

                char *result = (char *)malloc(sizeof(char) * num_servers * mem_size);

                for (int i = 0; i < num_servers; i++)
                {
                    char *temp;
                    pthread_join(readers[i], (void **)&temp);
                    strcat(result, temp);
                }

                char my_mem[mem_size];
                read_memory(args->memory, my_mem, 0, mem_size - 1);

                // Como ele eh o id mais alto sempre, vai ser o ultimo a concatenar
                strcat(result, my_mem);
                write_log(result);

                free(result);
            }
            else
            {
                char my_mem[mem_size];
                read_memory(args->memory, my_mem, 0, mem_size - 1);

                // Como ele eh o id mais alto sempre, vai ser o ultimo a concatenar
                write_log(my_mem);
            }
        }
    }
}