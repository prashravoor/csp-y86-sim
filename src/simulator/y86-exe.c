#include "y86-exe.h"

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


void instruction_fetch()
{

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
