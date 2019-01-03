#include <stdio.h>

#define MAX_MEMORY 32768
#define REG_SIZE 0xF

typedef unsigned char byte_t;

typedef struct {
    int cur;
    int max;
    byte_t* contents;
} memory_t;