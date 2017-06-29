#include "ArgumentPasser.h"

#include <string.h>
#include <iostream>

#include "ConsoleOutput/ConsoleOutput.h"

ArgumentPasser::ArgumentPasser()
{
}

ArgumentPasser::~ArgumentPasser()
{
}

bool ArgumentPasser::AddOption(string&& opt)
{
    return RecognizedOptions.insert(opt).second;
}

void ArgumentPasser::badargs() const
{
    ConsoleOutput::print("Type -h or --help for help.", ConsoleOutput::QUESTION);
}
void ArgumentPasser::multiple_encrypt_decrypt_options() const
{
    ConsoleOutput::print("Option -c or -d specified multiple times or both. Quit.", ConsoleOutput::ERROR);
}
bool ArgumentPasser::consumeArgument(string&& arg, Settings* s) const
{
    if (arg.length() < 1)
        return false;

    if (arg[0] == '-')
    {
        if (arg.length() >= 3 && arg[0] == arg[1])
        {//-- option
            if (arg == "--check")
                s->CheckForUnEncryptedFiles = true;
            else if(arg == "--find")
                s->find_encrypted = true;
            else if(arg == "--force")
                s->EncryptAllExtensions = true;
            else if (arg == "--help")
                s->printHelp = true;
            else if (arg == "--no-delete" || arg == "--no-del")
                s->no_delete = true;
            else if (arg == "--test" || arg == "--tests")
                s->runTests = true;
            else if (arg == "--version" || arg == "--ver")
                s->printVersion = true;
            else
            {
                print_unrecognized_option(arg);
                return false;
            }
        }
        else if(arg.length() >= 2 && arg[0] == '-')
        {//- option
            return consumeSingleArgument(arg, s);
        }
        else
        {
            print_unrecognized_option(arg);
            return false;
        }
    }
    else
    {
        print_unrecognized_option(arg);
        return false;
    }
    return true;
}

bool ArgumentPasser::consumeSingleArgument(const string& arg, Settings* s) const
{
    for (size_t i = 1; i < arg.length(); i++)
    {
        if (arg[i] == 'c')
        {
            if(s->encrypt_or_decrypt_option_found)
            {
                multiple_encrypt_decrypt_options();
                s->SetExitProgram_And_ExitCode(1);
                return false;
            }
            s->encrypt = true;
            s->encrypt_or_decrypt_option_found = true;
        }
        else if (arg[i] == 'd')
        {
            if(s->encrypt_or_decrypt_option_found)
            {
                multiple_encrypt_decrypt_options();
                s->SetExitProgram_And_ExitCode(1);
                return false;
            }
            s->encrypt = false;
            s->encrypt_or_decrypt_option_found = true;
        }
        else if (arg[i] == 'h')
            s->printHelp = true;
        else if (arg[i] == 'r')
            s->recursive = true;
        else if (arg[i] == 'v')
            s->verbose = true;
        else
        {
            string text = "-x";
            text[1] = arg[i];            
            print_unrecognized_option(text);
            return false;
        }
    }
    return true;
}

void ArgumentPasser::printhelp() const
{

}

bool ArgumentPasser::ParseArguments(char** args, int argc, Settings* s)
{
    if (argc <= 1 || !s)
    {
        badargs();
        return false;
    }
    
    bool LastOptionFound = false;
    //skip first one because it is path
    for (int i = 1; i < argc; i++)
    {
        string arg(args[i]);
        if(LastOptionFound || (arg.length() > 0 && arg[0] != '-'))
        {//found argument that is not starting with '-'
            LastOptionFound = true;
            s->AddFileName(move(arg));
            continue;
        }
                
        if (!consumeArgument(move(arg), s))
            return false;
    }    
    return true;
}

inline void ArgumentPasser::print_unrecognized_option(const string& opt) const
{
    ConsoleOutput::print("Unrecognized option '" + opt + "'. Quit.", ConsoleOutput::ERROR);
}