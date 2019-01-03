#ifndef ENUMS_H
#define ENUMS_H

/* REG_NONE is a special one to indicate no register */
enum reg_names_t
{
    R_RAX = 0,
    R_RCX,
    R_RDX,
    R_RBX,
    R_RSP,
    R_RBP,
    R_RSI,
    R_RDI,
    R_R8,
    R_R9,
    R_R10,
    R_R11,
    R_R12,
    R_R13,
    R_R14,
    R_NONE = 0xF,
    R_ERR
};

enum pipeline_statges_t
{
    P_IF = 0,
    P_DE,
    P_EX,
    P_ME,
    P_WB,
    P_ERR
};

/* Different argument types */
enum argument_type_t
{
    A_REG,
    A_MEM,
    A_IMM,
    A_NO_ARG
};

/* Different instruction types */
enum ins_type_t
{
    I_HALT,
    I_NOP,
    I_RRMOV,
    I_IRMOV,
    I_RMMOV,
    I_MRMOV,
    I_ALU,
    I_JMP,
    I_CALL,
    I_RET,
    I_PUSH,
    I_POP,
    I_ALUOP,
    I_NONE
};

/* Different ALU operations */
enum alu_type_t
{
    ALU_ADD,
    ALU_SUB,
    ALU_AND,
    ALU_XOR,
    ALU_NONE
};

/* Default function code */
enum func_t
{
    F_NONE
};

/* Different Jump conditions */
enum cond_type_t
{
    C_YES,
    C_LE,
    C_L,
    C_E,
    C_NE,
    C_GE,
    C_G
};

#endif
