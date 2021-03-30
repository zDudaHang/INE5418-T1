#include <stdlib.h>
#include "client_request.h"

enum Client_Status
{
    ERROR,
    QUIT
};

void print_commands();
void print_welcome_message();
int verify_user_request(char type, char *request, configs_t configs);

void print_welcome_message()
{
    printf(
        "Welcome!\n"
        "==========================\n");
    print_commands();
}

void print_commands()
{
    printf(
        "To read, type: r <start_position> <size>\n"
        "To write, type: w <start_position> <text> <size>\n"
        "To quit, type: q\n"
        "To see the commands, type: h\n");
}

int verify_user_request(char type, char *request, configs_t configs)
{
    switch (type)
    {
    case 'r':
        return user_read_request(request, configs);
    case 'w':
        return user_write_request(request, configs);
    case 'q':
        return QUIT;
    case 'h':
        print_commands();
        return 0;
    default:
        printf("[ERROR] Wrong request. Type h to see the formats.\n");
        return ERROR;
    }
}