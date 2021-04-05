#include "libs/client.h"

void print_welcome_message();
void print_commands();

int main()
{
    configs_t configs = read_configs();
    char input[REQ_SIZE];
    char request[REQ_SIZE];
    int result;

    print_welcome_message();

    while (1)
    {
        fgets(input, REQ_SIZE, stdin);
        strcpy(request, input);
        if (verify_user_request(input[0], request, configs) == QUIT)
        {
            break;
        }
    }
    return 0;
}