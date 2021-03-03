#include <stdio.h>
#include <assert.h>
#include "../shared_memory.h"

// MONO-THREAD SHARED MEMORY LIB TEST
int main() {
    shared_memory_element_t shared_memory_element memory[10];

    init_memory(memory, 10);

    for (int i = 0; i < 10; i++) {
        assert(memory[i].value == '\0');
    }

    write(memory, 'a', 0);
    write(memory, 'l', 1);
    write(memory, 'o', 2);

    assert(memory[0].value == 'a');
    assert(memory[1].value  == 'l');
    assert(memory[2].value  == 'o');

    char result[3];
    read(memory, result, 0, 2);

    assert(result[0] == 'a');
    assert(result[1] == 'l');
    assert(result[2] == 'o');

    return 0;
}
