#include "y86-sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

memory_t instructions;
byte_t registers[REG_SIZE];

void load();
void startExec();
void enablePipeline();
void singleStepMode();
void dumpRegisters();
void dumpPipelineRegs();
void dumpMemory();
void restart();
void showSource();
void executeNext();
void initialize();
void initialize_memory(memory_t *ptr, int size);
byte_t read_byte(memory_t *mem);
void write_byte(memory_t *mem, byte_t b);

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
        printf("3.  Enable PipeLine\n");
        printf("4.  Enter Single Step Mode\n");
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
            enablePipeline();
            break;
        case 4:
            singleStepMode();
            break;
        case 5:
            dumpRegisters();
            break;
        case 6:
            dumpPipelineRegs();
            break;
        case 7:
            dumpMemory();
            break;
        case 8:
            restart();
            break;
        case 9:
            showSource();
            break;
        case 10:
            executeNext();
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
    bzero(&registers, sizeof(registers));
    initialize_memory(&instructions, MAX_MEMORY);
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
    instructions.max = count;
    instructions.cur = 0;
    while (instructions.cur < instructions.max)
    {
        printf("%.2x ", read_byte(&instructions));
    }
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
    /*byte_t b = mem->contents[mem->cur];
    mem->cur += 1;
    return b;
    */
   return mem->contents[mem->cur++];
}

void startExec()
{
    // Start execution
}

void enablePipeline()
{
    // Enable Pipeline
}
void singleStepMode()
{
    // Single Step mode
}

void dumpRegisters()
{
    printf("Register Contents:\n");
}

void dumpPipelineRegs()
{
    printf("Pipeline Register Contents:\n");
}
void dumpMemory()
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

void executeNext()
{
    int numIns = 1;
    printf("Enter the number of instructions to execute: ");
    scanf("%d", &numIns);
}