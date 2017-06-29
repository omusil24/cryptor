#ifndef SHRED_H
#define SHRED_H


#include <vector>
#include <string>

using namespace std;

/*
 * @brief Rewritten variation of shred application available on linux
 * It overwrites the file with zeros at the end
 * Implementation of
 * shred -fuz {}
 */
class Shred {

    enum class MESSAGETYPE {
        FAIL,
        OK
    };

public:

    enum PassType {
        Predefined_Patterns,
        Random,
        Zeros        
    };
    /**
     * @brief Initialized with verbose set as false
     * @param args
     */
    Shred(uint8_t numberOfPasses_ = 3);
    ~Shred();

private:
    /**
     * @brief Overwrites entire file with zeros.
     * Position within file is expected to be at the beginning of file
     * @param file_descriptor Descriptor of file that is opened in write mode.
     * If descriptor is not valid, function returns false
     * @param generator_type
     * @param given_file_size
     * @return Whether any error happened during the action or all is ok
     */
    inline bool do_pass(int file_descriptor, PassType generator_type,
            uint64_t given_file_size) const;

    void print(string&& what, bool t = true) const;

public:
    bool& verbose();
    bool WipeFile(const string& AbsolutePath);

private:
    uint8_t numberOfPasses;
    bool verbose_private;
};


#endif /* SHRED_H */

