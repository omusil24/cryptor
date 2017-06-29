#include "MemoryWiper.h"

#include <cinttypes>

void MemoryWiper::ClearVariable(int* var)
{
    *var = 0;
}

void MemoryWiper::ClearVariable(string* var)
{
    uint32_t length = var->length();
    for (uint32_t i = 0; i < length; i++)
    {
        (*var)[i] = 'x';
    }
}

void MemoryWiper::ClearVariable(unsigned char* arr, uint16_t len)
{
    for (uint32_t i = 0; i < len; i++)
        arr[i] = 0xab;
}