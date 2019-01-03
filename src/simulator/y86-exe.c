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
        value = value | (temp << (i == 1 ? 0 : 8)); // For the first byte, don't shift
    }

    return value;
}

void reset_pipeline(int pipe)
{
    if (pipe >= P_IF && pipe <= P_WB)
    {
        pipeline_t *p = &pipeline_registers[pipe];
        p->func_type = F_NONE;
        p->ins_type = I_NONE;
        p->is_full = 0;
        p->regA = R_NONE;
        p->regB = R_NONE;
        p->valA = 0;
        p->valB = 0;
        p->valC = 0;
        p->valE = 0;
        p->valM = 0;
    }
}

int error = 0;
extern int cur_ins;
extern simulator_t simulator;

void instruction_fetch()
{
    int regA, regB, ins_type, func_type;
    reg_t valC;

    if (error)
    {
        printf("The simulator is in error state!\n");
        return;
    }

    if (!pipeline_enabled)
    {
        if (cur_ins != P_IF)
            return;
    }

    if (pipeline_registers[P_DE].is_full)
    {
        printf("IF stalled due to DE\n");
        return;
    }

    reset_pipeline(P_IF);
    pipeline_registers[P_DE].is_full = 0;
    byte_t code = read_byte(instructions);
    if (0 == code)
    {
        printf("Reached end of instructions\n");
        return;
    }

    ins_type = get_higher_nibble(code);
    regA = R_NONE;
    regB = R_NONE;
    valC = 0;
    func_type = F_NONE;

    switch (ins_type)
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
        byte_t registers = read_byte(instructions);
        regA = get_higher_nibble(registers);
        regB = get_lower_nibble(registers);
        func_type = get_lower_nibble(ins_type);

        printf("Got registers: %u, %u\n", regA, regB);
        switch (func_type)
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
            error = 1;
            break;
        }
        break;
    }
    case I_IRMOV:
    {
        byte_t registers = read_byte(instructions);
        regB = get_lower_nibble(registers);
        valC = read_number(instructions);
        printf("Immediate To Register Move: Register: %u, Value: %llX\n", regB, valC);
        break;
    }
    case I_RMMOV:
    {
        byte_t registers = read_byte(instructions);
        regA = get_higher_nibble(registers);
        regB = get_lower_nibble(registers);
        valC = read_number(instructions);

        printf("Register to Memory Move. Reg A: %u, Reg B: %u, Value: %llX\n", regA, regB, valC);
        break;
    }
    case I_MRMOV:
    {
        byte_t registers = read_byte(instructions);
        regA = get_higher_nibble(registers);
        regB = get_lower_nibble(registers);
        valC = read_number(instructions);

        printf("Memory to Register move. Reg A: %u, Reg B: %u, Value: %llX\n", regA, regB, valC);
        break;
    }
    case I_ALU:
    {
        byte_t registers = read_byte(instructions);
        regA = get_higher_nibble(registers);
        regB = get_lower_nibble(registers);
        func_type = get_lower_nibble(ins_type);

        printf("ALU Operation. Type: %u, RegA: %u, RegB: %u\n", func_type, regA, regB);
        break;
    }
    case I_JMP:
    {
        func_type = get_lower_nibble(ins_type);
        valC = read_number(instructions);

        printf("Jump. Type: %u, Value: %llX\n", func_type, valC);
        break;
    }
    case I_CALL:
    {
        valC = read_number(instructions);
        printf("Call. Value: %llX\n", valC);
        break;
    }
    case I_RET:
    {
        printf("Return\n");
        break;
    }
    case I_PUSH:
    {
        byte_t registers = read_byte(instructions);
        regA = get_higher_nibble(registers);
        printf("Push. Register: %u\n", regA);
        break;
    }
    case I_POP:
    {
        byte_t registers = read_byte(instructions);
        regA = get_higher_nibble(registers);
        printf("Pop. Register: %u\n", regA);
        break;
    }
    default:
        printf("Invalid Instruction: %u\n", ins_type);
        error = 1;
    }

    if (!error)
    {
        pipeline_registers[P_DE].is_full = 1;
        pipeline_registers[P_DE].ins_type = ins_type;
        pipeline_registers[P_DE].func_type = func_type;
        pipeline_registers[P_DE].regA = regA;
        pipeline_registers[P_DE].regB = regB;
        pipeline_registers[P_DE].valC = valC;
    }
}

void instruction_decode()
{
    pipeline_t *s = &pipeline_registers[P_DE];
    pipeline_t *d = &pipeline_registers[P_EX];

    if (error)
    {
        printf("The simulator is in error state!\n");
        return;
    }

    if (!pipeline_enabled)
    {
        if (cur_ins != P_DE)
            return;
    }

    if (!s->is_full)
    {
        printf("Decode stalled\n");
        return;
    }

    if (d->is_full)
    {
        printf("Decode stalled due to execute\n");
        return;
    }

    s->is_full = 0;

    // For call and return, stack pointer is implied
    if (s->ins_type == I_CALL || s->ins_type == I_RET)
    {
        if (s->ins_type == I_RET)
        {
            s->regA = R_RSP;
        }
        s->regB = R_RSP;
    }

    // For PUSH and POP, stack pointer is implied
    if (s->ins_type == I_PUSH || s->ins_type == I_POP)
    {
        if (s->ins_type == I_POP)
        {
            s->regA = R_RSP;
        }
        s->regB = R_RSP;
    }

    if (s->regA != R_NONE)
        d->valA = simulator.registers[s->regA];

    if (s->regB != R_NONE)
        d->valB = simulator.registers[s->regB];

    d->ins_type = s->ins_type;
    d->func_type = s->func_type;
    d->regA = s->regA;
    d->regB = s->regB;
    d->valC = s->valC;
    d->is_full = 1;

    reset_pipeline(P_DE);
}

void instruction_execute()
{
    pipeline_t *s = &pipeline_registers[P_EX];
    pipeline_t *d = &pipeline_registers[P_ME];

    if (error)
    {
        printf("The simulator is in error state!\n");
        return;
    }

    if (!pipeline_enabled)
    {
        if (cur_ins != P_EX)
            return;
    }

    if (!s->is_full)
    {
        printf("Execute stalled\n");
        return;
    }

    if (d->is_full)
    {
        printf("Execute stalled due to ME\n");
        return;
    }

    switch (s->ins_type)
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
        s->valE = s->valB + s->valA;
        int cnd_met = condition_satisfied(simulator.zero_flag, simulator.sign_flag, simulator.of_flag, s->func_type);
        if (!cnd_met)
        {
            d->regB = R_NONE;
        }
        break;
    }
    case I_IRMOV:
    {
        if (s->regB == R_NONE)
        {
            error = 1;
            printf("Invalid register specified!");
            return;
        }

        // simulator.registers[s->regB] = s->valC;
        break;
    }
    case I_RMMOV:
    {
        s->valE = s->valB + s->valC;
        break;
    }
    case I_MRMOV:
    {
        s->valE = s->valC + s->valB;
        break;
    }
    case I_ALU:
    {
        switch (s->func_type)
        {
        case ALU_ADD:
            s->valE = s->valA + s->valB;
            break;
        case ALU_AND:
            s->valE = s->valA & s->valB;
            break;
        case ALU_SUB:
            s->valE = s->valA - s->valB;
            break;
        case ALU_XOR:
            s->valE = s->valA ^ s->valB;
            break;
        default:
            printf("Invalid ALU operation: %d\n", s->func_type);
            error = 1;
            break;
        }
        set_cond_codes(s->valA, s->valB, s->valE, s->func_type);
        break;
    }
    case I_JMP:
    {
        int jmp = condition_satisfied(simulator.zero_flag, simulator.sign_flag, simulator.of_flag, s->func_type);
        if (jmp)
        {
            printf("Jump instruction satisfied: New PC address: %llX\n", s->valC);
            instructions->cur = s->valC;
        }
        break;
    }
    case I_CALL:
    {
        s->valE = s->valB - 8;
        break;
    }
    case I_RET:
    {
        s->valA = s->valA + 8;
        break;
    }
    case I_PUSH:
    {
        s->valE = s->valB - 8;
        break;
    }
    case I_POP:
    {
        s->valE = s->valB + 8;
        break;
    }
    default:
        printf("Invalid Instruction: %u\n", s->ins_type);
        error = 1;
    }

    d->ins_type = s->ins_type;
    d->func_type = s->func_type;
    d->regA = s->regA;
    d->regB = s->regB;
    d->valA = s->valA;
    d->valB = s->valB;
    d->valC = s->valC;
    d->valE = s->valE;
    d->valM = s->valM;
    d->is_full = 1;

    reset_pipeline(P_EX);
}

void set_cond_codes(reg_t valA, reg_t valB, reg_t valE, int type)
{
    simulator.zero_flag = !valE;
    simulator.sign_flag = 0 > ((long long)valE);
    switch (type)
    {
    case ALU_ADD:
        simulator.of_flag = (((long long)valA < 0) == ((long long)valB < 0)) &&
                            (((long long)valE < 0) != ((long long)valA < 0));
        break;
    case ALU_SUB:
        simulator.of_flag = (((long long)valA > 0) == ((long long)valB < 0)) &&
                            (((long long)valE < 0) != ((long long)valB < 0));
        break;
    case ALU_AND:
    case ALU_XOR:
        simulator.of_flag = 0;
        break;
    default:
        simulator.of_flag = 0;
    }
}

void instruction_memory()
{
    pipeline_t *s = &pipeline_registers[P_ME];
    pipeline_t *d = &pipeline_registers[P_WB];

    if (error)
    {
        printf("The simulator is in error state!\n");
        return;
    }

    if (!pipeline_enabled)
    {
        if (cur_ins != P_ME)
            return;
    }

    if (!s->is_full)
    {
        printf("Memory Stage stalled\n");
        return;
    }

    if (d->is_full)
    {
        printf("Memory Stage stalled due to WB\n");
        return;
    }

    switch (s->ins_type)
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
        break;
    }
    case I_IRMOV:
    {
        break;
    }
    case I_RMMOV:
    {
        printf("Setting memory loc %llX to %llX\n", s->valE, s->valA);
        write_memory(&simulator.memory, s->valA, s->valE);
        break;
    }
    case I_MRMOV:
    {
        printf("Setting register %d to contents of memory %llX\n", s->regA, s->valE);
        s->valA = read_memory(&simulator.memory, s->valE);
        break;
    }
    case I_ALU:
    {
        break;
    }
    case I_JMP:
    {
    }
    case I_CALL:
    {
        printf("Call-ed, write value %llX to memory location %llX\n", (reg_t)instructions->cur, s->valE);
        write_memory(&simulator.memory, (reg_t)instructions->cur, s->valE);
        break;
    }
    case I_RET:
    {
        printf("Ret-ed, read value from memory location %llX\n", s->valA);
        s->valM = read_memory(&simulator.memory, s->valA);
        break;
    }
    case I_PUSH:
    {
        printf("Push-ed, write value %llX to memory location %llX\n", s->valE, s->valB);
        write_memory(&simulator.memory, (reg_t)instructions->cur, s->valE);
        break;
    }
    case I_POP:
    {
        printf("Pop-ed, read value from memory location %llX\n", s->valA);
        s->valM = read_memory(&simulator.memory, s->valA);
        break;
    }
    default:
        printf("Invalid Instruction: %u\n", s->ins_type);
        error = 1;
    }

    d->ins_type = s->ins_type;
    d->func_type = s->func_type;
    d->regA = s->regA;
    d->regB = s->regB;
    d->valA = s->valA;
    d->valB = s->valB;
    d->valC = s->valC;
    d->valE = s->valE;
    d->valM = s->valM;
    d->is_full = 1;

    reset_pipeline(P_ME);
}

void instruction_write()
{
    pipeline_t *s = &pipeline_registers[P_WB];
    //pipeline_t *d = &pipeline_registers[P_WB];

    if (error)
    {
        printf("The simulator is in error state!\n");
        return;
    }

    if (!pipeline_enabled)
    {
        if (cur_ins != P_WB)
            return;
    }

    if (!s->is_full)
    {
        printf("WriteBack Stage stalled\n");
        return;
    }

    /*if (d->is_full)
    {
        printf("Memory Stage stalled due to WB\n");
        return;
    }*/

    switch (s->ins_type)
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
        if (s->regB != R_NONE)
        {
            simulator.registers[s->regB] = s->valE;
        }
        break;
    }
    case I_IRMOV:
    {
        if (s->regB != R_NONE)
        {
            simulator.registers[s->regB] = s->valE;
        }
        break;
    }
    case I_RMMOV:
    {
        break;
    }
    case I_MRMOV:
    {
        simulator.registers[s->regA] = s->valA;
        break;
    }
    case I_ALU:
    {
        simulator.registers[s->regB] = s->valE;
        break;
    }
    case I_JMP:
    {
    }
    case I_CALL:
    {
        simulator.registers[R_RSP] = s->valE;
        break;
    }
    case I_RET:
    {
        simulator.registers[R_RSP] = s->valE;
        break;
    }
    case I_PUSH:
    {
        simulator.registers[R_RSP] = s->valE;
        break;
    }
    case I_POP:
    {
        simulator.registers[R_RSP] = s->valE;
        simulator.registers[s->regA] = s->valM;
        break;
    }
    default:
        printf("Invalid Instruction: %u\n", s->ins_type);
        error = 1;
    }

    /*d->ins_type = s->ins_type;
    d->func_type = s->func_type;
    d->regA = s->regA;
    d->regB = s->regB;
    d->valA = s->valA;
    d->valB = s->valB;
    d->valC = s->valC;
    d->valE = s->valE;
    d->valM = s->valM;
    d->is_full = 1;*/

    reset_pipeline(P_ME);
}

void update_pc()
{
    // PC needs to change only when there is a branch
}

/* Branch logic */
int condition_satisfied(byte_t zf, byte_t sf, byte_t of, int cond)
{
    int jump = 0;

    switch (cond)
    {
    case C_YES:
        jump = 1;
        break;
    case C_LE:
        jump = (sf ^ of) | zf;
        break;
    case C_L:
        jump = sf ^ of;
        break;
    case C_E:
        jump = zf;
        break;
    case C_NE:
        jump = zf ^ 1;
        break;
    case C_GE:
        jump = sf ^ of ^ 1;
        break;
    case C_G:
        jump = (sf ^ of ^ 1) & (zf ^ 1);
        break;
    default:
        jump = 1;
        break;
    }
    return jump;
}

reg_t read_memory(memory_t *mem, int start_loc)
{
    int i = 0;
    reg_t value;

    for (i = 0; i < 8; ++i)
    {
        byte_t b = mem->contents[start_loc++];
        reg_t tmp = b;
        tmp = (tmp << i * 8);
        value = value | tmp;
    }

    return value;
}

void write_memory(memory_t *mem, reg_t value, int start_loc)
{
    int i = 0;
    byte_t b;
    for (i = 0; i < 8; ++i)
    {
        b = (value & 0xFF);
        value = value >> 8;
        mem->contents[start_loc++] = b;
    }
}