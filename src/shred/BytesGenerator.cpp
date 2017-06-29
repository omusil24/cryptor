#include "BytesGenerator.h"

#include <iostream>


#include "../Functions.h"
#include "Shred.h"

BytesGenerator::BytesGenerator(Shred::PassType gen_type_)
   :block_size(51200),
    gen_type(gen_type_)
{    
}
BytesGenerator::~BytesGenerator()
{    
}

size_t BytesGenerator::BlockSize() const
{
    return block_size;
}    
unsigned char* BytesGenerator::GetBlock() const
{
    return GetBlock(block_size);
}
unsigned char* BytesGenerator::GetBlock(uint64_t Exact_size) const
{
    size_t array_size = sizeof(Patterns) / sizeof(Patterns[0]);
    
    uint16_t index = Functions::GetRandomNumber(0, array_size - 1);
    uint16_t index_in_array = 0;
    
    unsigned char* buff = new unsigned char[Exact_size];
    
    while(index_in_array < Exact_size)
    {
        index = Functions::GetRandomNumber(0, array_size - 1);
        
        if(gen_type == Shred::PassType::Zeros)
            buff[index_in_array] = 0x00;
        else if(gen_type == Shred::PassType::Predefined_Patterns)
            buff[index_in_array] = Patterns[index];
        else
        {
            int* i = new int[5];
            i[0] = 9;
            
        }
        index_in_array ++;
    }
    return buff;    
}