#ifndef Y86_EXE_H
#define Y86_EXE_H

#include "y86-sim.h"

extern int pipeline_enabled;
extern simulator_t simulator;
extern memory_t* instructions;

#define PIPELINE_STAGES 5

typedef struct {
    int is_full; // Indicates whether or not to process
    int ins_type;
    int regA;
    int regB;
    int func_type;
    reg_t valA;
    reg_t valB;
    reg_t valC;
    reg_t valE;
    reg_t valM;
} pipeline_t;

pipeline_t pipeline_registers[PIPELINE_STAGES];

byte_t read_byte(memory_t *mem);

void write_byte(memory_t *mem, byte_t b);

unsigned get_higher_nibble(byte_t b);

unsigned get_lower_nibble(byte_t b);

reg_t read_number(memory_t* mem);

reg_t read_memory(memory_t* mem, int start_log);

void write_memory(memory_t* mem, reg_t value, int start_loc);

void reset_pipeline(int p);

void instruction_fetch();

void instruction_decode();

void instruction_execute();

void instruction_memory();

void instruction_write();

void update_pc();

int condition_satisfied(byte_t zf, byte_t sf, byte_t of, int cond);

void set_cond_codes(reg_t valA, reg_t valB, reg_t valE, int type);

#endif
