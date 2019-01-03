#include "y86-sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "y86-exe.h"
#include "limits.h"
#include "enums.h"

simulator_t simulator;
memory_t instructions;
int pipeline_enabled = 0;
int step_mode = 0;

void load();
void startExec();
void toggle_pipeline();
void toggle_step_mode();
void dump_registers();
void dump_pipeline_regs();
void dump_memory();
void restart();
void showSource();
void execute_next();
void initialize();
void initialize_memory(memory_t *ptr, int size);
byte_t read_byte(memory_t *mem);
void write_byte(memory_t *mem, byte_t b);

int run_program(int numSteps);

int main()
{
    printf("Y86 Instruction Set Simulator --- Begin\n");
    initialize();

    while (1)
    {
        int choice = 0;
        printf("Execution:\n");
        printf("1.  Load .yo file\n");
        printf("2.  Start Execution\n");
        printf("3.  Toggle PipeLine\n");
        printf("4.  Toggle Single Step Mode\n");
        printf("5.  Display Register Contents\n");
        printf("6.  Show Pipeline Register Contents\n");
        printf("7.  Dump Memory\n");
        printf("8.  Restart Execution\n");
        printf("9.  Show Source Program\n");
        printf("10. Execute Next Instructions\n");
        printf("11. Exit\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);
        if (choice < 1 || choice > 11)
        {
            printf("Invalid choice, Try Again\n");
            continue;
        }

        switch (choice)
        {
        case 1:
            load();
            break;
        case 2:
            startExec();
            break;
        case 3:
            toggle_pipeline();
            break;
        case 4:
            toggle_step_mode();
            break;
        case 5:
            dump_registers();
            break;
        case 6:
            dump_pipeline_regs();
            break;
        case 7:
            dump_memory();
            break;
        case 8:
            restart();
            break;
        case 9:
            showSource();
            break;
        case 10:
            execute_next();
            break;
        case 11:
            return 0;
            break;
        }
    }

    return 0;
}

void initialize()
{
    bzero(&instructions, sizeof(memory_t));
    bzero(&simulator, sizeof(simulator));
    initialize_memory(&simulator.memory, MAX_INS_MEMORY);
    initialize_memory(&simulator.memory, MAX_MEMORY);
}

void initialize_memory(memory_t *ptr, int size)
{
    if (ptr->contents)
    {
        free(ptr->contents);
        bzero(ptr, sizeof(memory_t));
    }
    ptr->contents = calloc(sizeof(unsigned char), size);
    ptr->max = size;
    ptr->size = 0;
    ptr->cur = 0;
}

void load()
{
    char filename[256];
    FILE *fp = NULL;

    bzero(filename, sizeof(filename));
    printf("Enter the fully-qualified-filename to load: ");
    scanf("%s", (char *)&filename);

    // Load and read file
    fp = fopen(filename, "rb");
    if (NULL == fp)
    {
        printf("The file %s does not exist!\n", filename);
        return;
    }

    byte_t buffer;

    int count = 0;
    while (fread(&buffer, 1, 1, fp))
    {
        //printf("%X ", buffer);
        write_byte(&instructions, buffer);
        ++count;
    }
    printf("Bytes read: %d\n", count);
    instructions.size = count;
    fclose(fp);
}

void write_byte(memory_t *mem, byte_t b)
{
    if (!mem || !mem->contents || mem->cur >= mem->max)
    {
        return;
    }
    mem->contents[mem->cur++] = b;
}

byte_t read_byte(memory_t *mem)
{
    if (!mem || !mem->contents || mem->cur >= mem->max)
    {
        return 0;
    }
   return mem->contents[mem->cur++];
}

void startExec()
{
    int numCycles = run_program(INT_MAX);
    printf("Total Execution Time: %d cycles\n", numCycles);
}

int run_program(int steps)
{
    int cycles = 0;
    int cur_steps = 0;
    // Start execution
    while (instructions.cur < instructions.size && cur_steps++ < steps)
    {
        instruction_fetch();
        instruction_decode();
        instruction_execute();
        instruction_memory();
        instruction_write();
        update_pc();
        cycles += 1;
    }

    return cycles;
}

void toggle_pipeline()
{
    if (pipeline_enabled) 
    {
        printf("Disabled the pipeline\n");
    } else {
        printf("Pipeline has been enabled\n");
    }
    pipeline_enabled = !pipeline_enabled;
}

void toggle_step_mode()
{
    // Single Step mode
    if (step_mode) 
    {
        printf("Disabled the Single Step Mode\n");
    } else {
        printf("Enabled Single Step Mode\n");
    }
    step_mode = !step_mode;
}

void print_register(int r, char* reg_contents)
{
    unsigned long long reg = simulator.registers[r];
    int i = 0;

    for(i = 0; i < 8; ++i)
    {
        // Get Most Significant byte of the number in each iteration
        snprintf(reg_contents + 2*i, 4, "%.2X ", (unsigned)((reg & 0xFF00000000000000) >> 56));
        reg = reg << 4;
    }
}

void dump_registers()
{
    char reg_contents[24];

    bzero(reg_contents, sizeof(reg_contents));
    printf("Register Contents:\n");

    print_register(R_RAX, reg_contents);
    printf("%s: %s\n", "RAX", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(R_RCX, reg_contents);
    printf("%s: %s\n", "RCX", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(R_RDX, reg_contents);
    printf("%s: %s\n", "RDX", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(R_RBX, reg_contents);
    printf("%s: %s\n", "RBX", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(R_RSP, reg_contents);
    printf("%s: %s\n", "RSP", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(R_RBP, reg_contents);
    printf("%s: %s\n", "RBP", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(R_RSI, reg_contents);
    printf("%s: %s\n", "RSI", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(R_RDI, reg_contents);
    printf("%s: %s\n", "RDI", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(R_R8, reg_contents);
    printf("%s: %s\n", "R08", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(R_R9, reg_contents);
    printf("%s: %s\n", "R09", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));
    
    print_register(R_R10, reg_contents);
    printf("%s: %s\n", "R10", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(R_R11, reg_contents);
    printf("%s: %s\n", "R11", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(R_R12, reg_contents);
    printf("%s: %s\n", "R12", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(R_R13, reg_contents);
    printf("%s: %s\n", "R13", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(R_R14, reg_contents);
    printf("%s: %s\n", "R14", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));
}

void dump_pipeline_regs()
{
    printf("Pipeline Register Contents:\n");
}
void dump_memory()
{
    printf("Memory Contents:\n");
}

void restart()
{
    // Restart Exec
}

void showSource()
{
    printf("Source file: ");
}

void execute_next()
{
    int numIns = 1;
    printf("Enter the number of instructions to execute: ");
    scanf("%d", &numIns);
}