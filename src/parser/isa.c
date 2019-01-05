#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "isa.h"

/* Bytes Per Line = Block size of memory */
#define BPL 32

char *shmptr = (void *) -1;

struct {
    char *name;
    int id;
} reg_table[REG_ERR+1] = 
{
    {"%eax",   REG_EAX},
    {"%ecx",   REG_ECX},
    {"%edx",   REG_EDX},
    {"%ebx",   REG_EBX},
    {"%esp",   REG_ESP},
    {"%ebp",   REG_EBP},
    {"%esi",   REG_ESI},
    {"%edi",   REG_EDI},
    {"----",  REG_ERR},
    {"----",  REG_ERR},
    {"----",  REG_ERR},
    {"----",  REG_ERR},
    {"----",  REG_ERR},
    {"----",  REG_ERR},
    {"----",  REG_ERR},
    {"----",  REG_NONE},
    {"----",  REG_ERR}
};


reg_id_t find_register(char *name)
{
    int i;
    for (i = 0; i < REG_NONE; i++)
        if (!strcmp(name, reg_table[i].name))
            return reg_table[i].id;
    return REG_ERR;
}

char *reg_name(reg_id_t id)
{
    if (id >= 0 && id < REG_NONE)
        return reg_table[id].name;
    else
        return reg_table[REG_NONE].name;
}

/* Is the given register ID a valid program register? */
int reg_valid(reg_id_t id)
{
    return id >= 0 && id < REG_NONE && reg_table[id].id == id;
}

instr_t instruction_set[] = 
{
    {"nop",    HPACK(I_NOP, F_NONE), 1, NO_ARG, 0, 0, NO_ARG, 0, 0 },
    {"halt",   HPACK(I_HALT, F_NONE), 1, NO_ARG, 0, 0, NO_ARG, 0, 0 },
    {"rrmovl", HPACK(I_RRMOVL, F_NONE), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    /* Conditional move instructions are variants of RRMOVL */
    {"cmovle", HPACK(I_RRMOVL, C_LE), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"cmovl", HPACK(I_RRMOVL, C_L), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"cmove", HPACK(I_RRMOVL, C_E), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"cmovne", HPACK(I_RRMOVL, C_NE), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"cmovge", HPACK(I_RRMOVL, C_GE), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"cmovg", HPACK(I_RRMOVL, C_G), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    /* arg1hi indicates number of bytes */
    {"irmovl", HPACK(I_IRMOVL, F_NONE), 10, I_ARG, 2, 8, R_ARG, 1, 0 },
    {"rmmovl", HPACK(I_RMMOVL, F_NONE), 10, R_ARG, 1, 1, M_ARG, 1, 0 },
    {"mrmovl", HPACK(I_MRMOVL, F_NONE), 10, M_ARG, 1, 0, R_ARG, 1, 1 },
    {"addl",   HPACK(I_ALU, A_ADD), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"subl",   HPACK(I_ALU, A_SUB), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"andl",   HPACK(I_ALU, A_AND), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"xorl",   HPACK(I_ALU, A_XOR), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    /* arg1hi indicates number of bytes */
    {"jmp",    HPACK(I_JMP, C_YES), 9, I_ARG, 1, 8, NO_ARG, 0, 0 },
    {"jle",    HPACK(I_JMP, C_LE), 9, I_ARG, 1, 8, NO_ARG, 0, 0 },
    {"jl",     HPACK(I_JMP, C_L), 9, I_ARG, 1, 8, NO_ARG, 0, 0 },
    {"je",     HPACK(I_JMP, C_E), 9, I_ARG, 1, 8, NO_ARG, 0, 0 },
    {"jne",    HPACK(I_JMP, C_NE), 9, I_ARG, 1, 8, NO_ARG, 0, 0 },
    {"jge",    HPACK(I_JMP, C_GE), 9, I_ARG, 1, 8, NO_ARG, 0, 0 },
    {"jg",     HPACK(I_JMP, C_G), 9, I_ARG, 1, 8, NO_ARG, 0, 0 },
    {"call",   HPACK(I_CALL, F_NONE),    9, I_ARG, 1, 8, NO_ARG, 0, 0 },
    {"ret",    HPACK(I_RET, F_NONE), 1, NO_ARG, 0, 0, NO_ARG, 0, 0 },
    {"pushl",  HPACK(I_PUSHL, F_NONE) , 2, R_ARG, 1, 1, NO_ARG, 0, 0 },
    {"popl",   HPACK(I_POPL, F_NONE) ,  2, R_ARG, 1, 1, NO_ARG, 0, 0 },
    {"iaddl",  HPACK(I_IADDL, F_NONE), 10, I_ARG, 2, 8, R_ARG, 1, 0 },
    {"leave",  HPACK(I_LEAVE, F_NONE), 1, NO_ARG, 0, 0, NO_ARG, 0, 0 },
    /* this is just a hack to make the I_POP2 code have an associated name */
    {"pop2",   HPACK(I_POP2, F_NONE) , 0, NO_ARG, 0, 0, NO_ARG, 0, 0 },

    /* For allocation instructions, arg1hi indicates number of bytes */
    {".byte",  0x00, 1, I_ARG, 0, 1, NO_ARG, 0, 0 },
    {".word",  0x00, 2, I_ARG, 0, 2, NO_ARG, 0, 0 },
    {".long",  0x00, 4, I_ARG, 0, 4, NO_ARG, 0, 0 },
    {NULL,     0   , 0, NO_ARG, 0, 0, NO_ARG, 0, 0 }
};

instr_t invalid_instr =
{"XXX",     0   , 0, NO_ARG, 0, 0, NO_ARG, 0, 0 };

instr_ptr find_instr(char *name)
{
    int i;
    for (i = 0; instruction_set[i].name; i++)
        if (strcmp(instruction_set[i].name,name) == 0)
            return &instruction_set[i];
    return NULL;
}

/* Return name of instruction given its encoding */
char *iname(int instr) {
    int i;
    for (i = 0; instruction_set[i].name; i++) {
        if (instr == instruction_set[i].code)
            return instruction_set[i].name;
    }
    return "<bad>";
}

instr_ptr bad_instr()
{
    return &invalid_instr;
}

