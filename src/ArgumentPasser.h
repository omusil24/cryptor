#ifndef ARGUMENTPASSER_H
#define ARGUMENTPASSER_H

#include <unordered_set>

#include "Crypto/Crypto.h"
#include "Settings/Settings.h"

using namespace std;

class cmdOptions;

class ArgumentPasser
{
    public:
        ArgumentPasser();
        virtual ~ArgumentPasser();
        
    private:
        void badargs() const;
        bool consumeArgument(string&& arg, Settings* s, const cmdOptions* options) const;
        bool consumeSingleArgument(const string& arg, Settings* s, const cmdOptions* options) const;
        bool found_enc_dec_option(bool encrypt, Settings* s) const;
        void multiple_encrypt_decrypt_options() const;
        void printhelp() const;
    public:
        /**
         * 
         * @param args
         * @param argc
         * @param c Pointer to crypto object to modify according to arguments
         * @return 
         */
        bool ParseArguments(char** args, int argc, Settings* s, const cmdOptions* options);
            
    protected:

    private:
        inline void print_unrecognized_option(const string& opt) const;
};

#endif // ARGUMENTPASSER_H
