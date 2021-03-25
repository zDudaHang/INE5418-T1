#include <string.h>
#include <math.h>

#define REQ_SIZE 50
#define SEPARATOR " "

typedef struct server_range
{
    unsigned int start;
    unsigned int end;
} server_range_t;

server_range_t verify_which_servers(int start, int end, unsigned int memory_size);

char * read_request(char request[REQ_SIZE], configs_t configs) {
    // Extract the start and end positions
    strtok(request, SEPARATOR); // r
    int start = atoi(strtok(NULL, SEPARATOR));
    strtok(NULL, SEPARATOR); // until
    int end = atoi(strtok(NULL, SEPARATOR));

    // Verify which servers we need to connect
    server_range_t range = verify_which_servers(start, end, configs.memory_size);

    // 
    return "Oi";
}

server_range_t verify_which_servers(int start, int end, unsigned int memory_size) {
    server_range_t range;
    range.start = floor(start / memory_size);
    range.end = floor(end / memory_size);
    return range;
}

void write_request(char input[REQ_SIZE], configs_t configs) {

}