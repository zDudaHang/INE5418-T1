#include "server_request.h"

char * verify_client_request(char type, char *request, configs_t configs, shared_memory_element_t * memory)
{
    switch (type)
    {
    case 'r':
        return client_read_request(request, configs, memory);
    case 'w':
        return 0;
    case 'q':
        return 0;
    case 'h':
        return 0;
    default:
        return 0;
    }
}