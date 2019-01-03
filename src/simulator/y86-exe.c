#include "y86-exe.h"
#include "enums.h"

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
    if (!mem || !mem->contents || mem->cur > mem->size)
    {
        return 0;
    }
    return mem->contents[mem->cur++];
}

unsigned get_higher_nibble(byte_t b)
{
    return (b & 0xF0) >> 4;
}

unsigned get_lower_nibble(byte_t b)
{
    return (b & 0xF);
}

// Read 8 bytes from memory, returned as a register value
reg_t read_number(memory_t *mem)
{
    reg_t value = 0;
    int i = 0;
    while (i++ < 8 && mem->cur < mem->size)
    {
        unsigned temp = read_byte(mem);
        value = value | (temp << ( i == 1 ? 0 : 8)); // For the first byte, don't shift
    }

    return value;
}

void instruction_fetch()
{
    byte_t code = read_byte(&instructions);

    unsigned ins = get_higher_nibble(code);
    switch (ins)
    {
    case I_HALT:
        printf("Halting the program\n");
        return;
        break;
    case I_NOP:
        printf("No Op\n");
        break;
    case I_CMOV:
    {
        byte_t registers = read_byte(&instructions);
        unsigned regA = get_higher_nibble(registers);
        unsigned regB = get_lower_nibble(registers);
        unsigned type = get_lower_nibble(ins);

        printf("Got registers: %u, %u\n", regA, regB);
        switch (type)
        {
        case C_YES:
            printf("Unconditional Move\n");
            break;
        case C_LE:
            printf("Move if LE\n");
            break;
        case C_L:
            printf("Move if L\n");
            break;
        case C_E:
            printf("Move if equal\n");
            break;
        case C_NE:
            printf("Move if not qual\n");
            break;
        case C_GE:
            printf("Move GE\n");
            break;
        case C_G:
            printf("Move G\n");
            break;
        default:
            printf("Invalid Condition!\n");
            break;
        }
        break;
    }
    case I_IRMOV:
    {
        byte_t registers = read_byte(&instructions);
        unsigned regA = get_lower_nibble(registers);
        reg_t value = read_number(&instructions);
        printf("Immediate To Register Move: Register: %u, Value: %llX\n", regA, value);
        break;
    }
    case I_RMMOV:
    {
        byte_t registers = read_byte(&instructions);
        unsigned regA = get_higher_nibble(registers);
        unsigned regB = get_lower_nibble(registers);
        reg_t value = read_number(&instructions);

        printf("Register to Memory Move. Reg A: %u, Reg B: %u, Value: %llX\n", regA, regB, value);
        break;
    }
    case I_MRMOV:
    {
        byte_t registers = read_byte(&instructions);
        unsigned regA = get_higher_nibble(registers);
        unsigned regB = get_lower_nibble(registers);
        reg_t value = read_number(&instructions);

        printf("Memory to Register move. Reg A: %u, Reg B: %u, Value: %llX\n", regA, regB, value);
        break;
    }
    case I_ALU:
    {
        byte_t registers = read_byte(&instructions);
        unsigned regA = get_higher_nibble(registers);
        unsigned regB = get_lower_nibble(registers);
        unsigned type = get_lower_nibble(ins);

        printf("ALU Operation. Type: %u, RegA: %u, RegB: %u\n", type, regA, regB);
        break;
    }
    case I_JMP:
    {
        unsigned type = get_lower_nibble(ins);
        reg_t value = read_number(&instructions);

        printf("Jump. Type: %u, Value: %llX\n", type, value);
        break;
    }
    case I_CALL:
    {
        reg_t value = read_number(&instructions);
        printf("Call. Value: %llX\n", value);
        break;
    }
    case I_RET:
    {
        printf("Return\n");
        break;
    }
    case I_PUSH:
    {
        byte_t registers = read_byte(&instructions);
        unsigned regA = get_higher_nibble(registers);
        printf("Push. Register: %u\n", regA);
        break;
    }
    case I_POP:
    {
        byte_t registers = read_byte(&instructions);
        unsigned regA = get_higher_nibble(registers);
        printf("Pop. Register: %u\n", regA);
        break;
    }
    default:
        printf("Invalid Instruction: %u\n", ins);
    }
}

void instruction_decode()
{
}

void instruction_execute()
{
}

void instruction_memory()
{
}

void instruction_write()
{
}

void update_pc()
{
}

void branch()
{
}
