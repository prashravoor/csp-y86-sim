#include "y86-sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "y86-exe.h"
#include "limits.h"
#include "enums.h"

simulator_t simulator;
memory_t *instructions;

static int instructions_loaded = 0;

int pipeline_enabled = 0;
int step_mode = 0;
int cur_ins = P_IF;

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

int run_program(int numSteps);

int main()
{
    printf("Y86 Instruction Set Simulator --- Begin\n");

    while (1)
    {
        int choice = 0;
        printf("\n%s\n\n", "**************Execution*****************");
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
        printf("11. Exit\n\n\n");

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
            initialize();
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
        default:
            printf("Invalid choice!\n");
        }
        printf("\n\n");
    }

    return 0;
}

void initialize()
{
    if (simulator.memory.contents)
    {
        free(simulator.memory.contents);
    }

    bzero(&simulator, sizeof(simulator));
    initialize_memory(&simulator.memory, MAX_MEMORY);
    instructions_loaded = 0;
    instructions = &simulator.memory;

    reset_pipeline(P_IF);
    reset_pipeline(P_DE);
    reset_pipeline(P_EX);
    reset_pipeline(P_ME);
    reset_pipeline(P_WB);
    cur_ins = P_IF;
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

    // Initialize Stack Pointer to 0x3000
    simulator.registers[R_RSP] = 0x3000;
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
    while (fread(&buffer, sizeof(buffer), 1, fp))
    {
        //printf("%X ", buffer);
        write_byte(instructions, buffer);
        ++count;
    }

    printf("Bytes read: %d\n", count);
    if (count <= 0)
    {
        printf("The instructions file is empty!\n");
    }
    else
    {
        instructions_loaded = 1;
    }

    instructions->size = count;
    instructions->cur = 0;
    fclose(fp);

}

void startExec()
{
    if (!instructions_loaded)
    {
        printf("You need to load a program first!\n");
        return;
    }

    int num_steps = step_mode ? 1 : INT_MAX;
    int numCycles = run_program(num_steps);
    printf("Total Execution Time: %d cycles\n", numCycles);
}

extern int error;
extern int complete;
int run_program(int steps)
{
    int cycles = 0;
    int cur_steps = 0;

    // Start execution
    while ((instructions->cur < instructions->size && cur_steps++ < steps) && !complete)
    {
        if (error)
        {
            printf("Detected error, stopping\n");
            break;
        }

        instruction_fetch();
        instruction_decode();
        instruction_execute();
        instruction_memory();
        instruction_write();

        cycles += 1;
        cur_ins = (cur_ins + 1) % P_ERR;
    }

    if(complete || instructions->cur >= instructions->size)
    {
        printf("Program Execution Complete!\n");
        reset_pipeline(P_IF);
        reset_pipeline(P_DE);
        reset_pipeline(P_EX);
        reset_pipeline(P_ME);
        reset_pipeline(P_WB);
        cur_ins = P_IF;
    }

    return cycles;
}

void toggle_pipeline()
{
    if (pipeline_enabled)
    {
        printf("Disabled the pipeline\n");
    }
    else
    {
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
    }
    else
    {
        printf("Enabled Single Step Mode\n");
    }
    step_mode = !step_mode;
}

void rvereseArray(char arr[], int start, int end) 
{ 
    while (start < end) 
    { 
        int temp = arr[start];  
        arr[start] = arr[end]; 
        arr[end] = temp; 
        start++; 
        end--; 
    }  
}  

void print_register(reg_t reg, char *reg_contents)
{
    int i = 0;

    for (i = 7; i >= 0; --i)
    {
        // Get Most Significant byte of the number in each iteration
        unsigned value = (reg_t)( (reg >> i*8) & 0xFF);
        snprintf(reg_contents + 3 * (7-i), 4, "%.2X ", value);
    }
}

void dump_registers()
{
    if (!instructions_loaded)
    {
        printf("You need to load a program first!\n");
        return;
    }

    char reg_contents[24];

    bzero(reg_contents, sizeof(reg_contents));
    printf("Register Contents:\n");

    print_register(simulator.registers[R_RAX], reg_contents);
    printf("%s: %s\n", "RAX", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(simulator.registers[R_RCX], reg_contents);
    printf("%s: %s\n", "RCX", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(simulator.registers[R_RDX], reg_contents);
    printf("%s: %s\n", "RDX", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(simulator.registers[R_RBX], reg_contents);
    printf("%s: %s\n", "RBX", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(simulator.registers[R_RSP], reg_contents);
    printf("%s: %s\n", "RSP", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(simulator.registers[R_RBP], reg_contents);
    printf("%s: %s\n", "RBP", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(simulator.registers[R_RSI], reg_contents);
    printf("%s: %s\n", "RSI", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(simulator.registers[R_RDI], reg_contents);
    printf("%s: %s\n", "RDI", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(simulator.registers[R_R8], reg_contents);
    printf("%s: %s\n", "R08", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(simulator.registers[R_R9], reg_contents);
    printf("%s: %s\n", "R09", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(simulator.registers[R_R10], reg_contents);
    printf("%s: %s\n", "R10", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(simulator.registers[R_R11], reg_contents);
    printf("%s: %s\n", "R11", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(simulator.registers[R_R12], reg_contents);
    printf("%s: %s\n", "R12", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(simulator.registers[R_R13], reg_contents);
    printf("%s: %s\n", "R13", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    print_register(simulator.registers[R_R14], reg_contents);
    printf("%s: %s\n", "R14", reg_contents);
    bzero(reg_contents, sizeof(reg_contents));

    printf("\n\nCondition Codes: ZF: %d, SF: %d, OF: %d\n", simulator.zero_flag, simulator.sign_flag, simulator.of_flag);
}

void dump_pipeline_regs()
{
    if (!instructions_loaded)
    {
        printf("You need to load a program first!\n");
        return;
    }

    print_pipeline(P_IF);
    print_pipeline(P_DE);
    print_pipeline(P_EX);
    print_pipeline(P_ME);
    print_pipeline(P_WB);
}

void dump_memory()
{
    if (!instructions_loaded)
    {
        printf("You need to load a program first!\n");
        return;
    }

    int start = 0, end = 0, i, j;
    printf("Memory Contents:\n");
    printf("Enter start address: (0 - %d)", simulator.memory.max);
    scanf("%d", &start);

    if (start < 0 || start >= simulator.memory.max)
    {
        printf("Invalid address entered!");
        return;
    }

    printf("Enter end address: (0 - %d)", simulator.memory.max);
    scanf("%d", &end);
    if (end <= 0 || end > simulator.memory.max)
    {
        printf("Invalid address entered!");
        return;
    }

    for (i = start; i < end;)
    {
        printf("Address (0x): %6X:  ", i);
        for (j = 0; j < 32 && i < end; ++j, ++i)
        {
            printf("%.2X ", simulator.memory.contents[i]);
        }
        printf("\n");
    }
}

void restart()
{
    if (!instructions_loaded)
    {
        printf("You need to load a program first!\n");
        return;
    }

    error = 0;

    // Restart Exec
    printf("Going back to instruction 1\n");
    instructions->cur = 0;
}

void showSource()
{
    int cur;
    if (!instructions_loaded)
    {
        printf("You need to load a program first!\n");
        return;
    }

    cur = instructions->cur;
    instructions->cur = 0;
    while (instructions->cur < instructions->size)
    {
        int i = 0;
        printf("0x%.4X: ", instructions->cur);
        while ((i < 16) && (instructions->cur < instructions->size))
        {
            printf("%.2X ", read_byte(instructions));
            ++i;
        }
        printf("\n");
    }

    instructions->cur = cur;
    printf("\n");
}

void execute_next()
{

    if ( error )
    {
        printf("Simulator is in error state!\n");
        return;
    }

    int num_cycles = 0;
    if (!instructions_loaded)
    {
        printf("You need to load a program first!\n");
        return;
    }

    int numIns = 1;
    printf("Enter the number of instructions to execute: ");
    scanf("%d", &numIns);
    num_cycles = run_program(numIns);
    printf("Executed %d cycles\n", num_cycles);
}
