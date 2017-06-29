#ifndef MEMORYWIPER_H
#define MEMORYWIPER_H

#include <string>

using namespace std;

/**
 * @brief Class used for clearing content of variables (memory)
 * @param var
 */
struct MemoryWiper
{
    static void ClearVariable(int* var);
    static void ClearVariable(string* var);
    static void ClearVariable(unsigned char* arr, uint16_t len);
};

#endif /* MEMORYWIPER_H */

