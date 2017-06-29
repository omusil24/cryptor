#ifndef ARGUMENTPASSER_H
#define ARGUMENTPASSER_H

#include <unordered_set>

#include "Crypto/Crypto.h"
#include "Settings/Settings.h"

using namespace std;

class ArgumentPasser
{
    public:
        ArgumentPasser();
        virtual ~ArgumentPasser();
        bool AddOption(string&& opt);

    private:
        void badargs() const;
        bool consumeArgument(string&& arg, Settings* s) const;
        bool consumeSingleArgument(const string& arg, Settings* s) const;
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
        bool ParseArguments(char** args, int argc, Settings* s);
            
    protected:

    private:
        inline void print_unrecognized_option(const string& opt) const;
           
        unordered_set<string> RecognizedOptions;
    public:
};

#endif // ARGUMENTPASSER_H
