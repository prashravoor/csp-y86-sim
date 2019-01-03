#ifndef Y86_EXE_H
#define Y86_EXE_H

#include "y86-sim.h"

extern int pipeline_enabled;
extern simulator_t simulator;
extern memory_t instructions;

#define PIPELINE_STAGES 5

reg_t pipeline_registers[PIPELINE_STAGES];

byte_t read_byte(memory_t *mem);

void write_byte(memory_t *mem, byte_t b);

void instruction_fetch();

void instruction_decode();

void instruction_execute();

void instruction_memory();

void instruction_write();

void update_pc();

void branch();

#endif
