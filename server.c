#include "libs/server.h"

#define NUM_CLIENTS 5


pthread_t threads[NUM_CLIENTS];
handler_args_t args[NUM_CLIENTS];

int main()
{
    configs_t configs = read_configs();
    shared_memory_element_t memory[configs.memory_size];

    init_memory(memory, configs.memory_size);

    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address, client_address;

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = configs.server_base_port + configs.server_range_index;

    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    listen(server_sockfd, NUM_CLIENTS);

    client_len = sizeof(client_address);

    pthread_t thread;
    handler_args_t args;
    while (1)
    {
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);

        args.client_sockfd = client_sockfd;
        args.configs = configs;
        args.memory = memory;
        pthread_create(&thread, NULL, handler, &args);
    }

    close(server_sockfd);
    return 0;
}
