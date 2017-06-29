#ifndef BYTESGENERATOR_H
#define BYTESGENERATOR_H

using namespace std;

#include "Shred.h"

/**
 * @brief Used to generate blocks of bytes of size <= BLOCK_SIZE based on either a give pattern
 * or randomnes
 */
struct BytesGenerator
{    
    const char Patterns[1] = { 0x00 };
    
    BytesGenerator(Shred::PassType gen_type);
    BytesGenerator(const BytesGenerator& b) = delete;
    ~BytesGenerator();
    
    
    size_t BlockSize() const;
    /**
     * 
     * @param Exact_size You can specify manually how big the returned block will be.
     * @return 
     */
    unsigned char* GetBlock(uint64_t Exact_size) const;
    /**
     * 
     * @return block of chars. The size of returned block is equal to \ref block_size
     */
    unsigned char* GetBlock() const;
    
private:    
    size_t block_size;
    Shred::PassType gen_type;
    
    unsigned char* rng;
};

#endif /* BYTESGENERATOR_H */

