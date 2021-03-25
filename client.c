#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#include "libs/file.h"
#include "libs/request.h"

void print_welcome_message();
void print_commands();

int main() {
    configs_t configs = read_configs();
    char input[REQ_SIZE];
    char request[REQ_SIZE];
    int result;

    print_welcome_message();

    while(1) {
        fgets(input, REQ_SIZE, stdin);

        char type = input[0];
        strcpy(request, input);
        if (type == 'r') {
            read_request(request, configs);
        } else if (type == 'w') {
            write_request(request, configs);
        } else if (type == 'q') {
            printf("EXITING...\n");
            break;
        } else if (type == 'h') {
            print_commands();
        } else {
            printf("[ERROR] Wrong request. Type h to see the formats.\n");
        }
    }
    return 0;
}

void print_welcome_message() {
    printf(
        "Welcome!\n"
        "==========================\n"
    );   
    print_commands();
}

void print_commands() {
    printf(
        "To read, type: r <start_position> until <end_position>\n"
        "To write, type: w <text> in <start_position>\n"
        "To quit, type: q\n"
        "To see the commands, type: h\n"
    );
}