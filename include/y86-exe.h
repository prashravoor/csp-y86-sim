#ifndef Y86_EXE_H
#define Y86_EXE_H

#include "y86-sim.h"

extern int pipeline_enabled;
extern simulator_t simulator;
extern memory_t instructions;

void instruction_fetch();

void instruction_decode();

void instruction_execute();

void instruction_memory();

void instruction_write();

void update_pc();

void branch();

#endif