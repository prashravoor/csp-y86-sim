#ifndef Y86_SIM_H
#define Y86_SIM_H

#include <stdio.h>

#define MAX_INS_MEMORY 8192
#define MAX_MEMORY 16384
#define REG_SIZE 0xF

typedef unsigned char byte_t;
typedef unsigned long long reg_t;

typedef struct {
    int cur;
    int max;
    int size;
    byte_t* contents;
} memory_t;

typedef struct {
    memory_t memory;
    reg_t registers[REG_SIZE];
    unsigned int pc;
    byte_t zero_flag;
    byte_t of_flag;
    byte_t sign_flag;
} simulator_t;

#endif
