#include "ArgumentPasser.h"

#include <string.h>
#include <iostream>

#include "ConsoleOutput/ConsoleOutput.h"
#include "cmdOptions/cmdOptions.h"

ArgumentPasser::ArgumentPasser()
{
}

ArgumentPasser::~ArgumentPasser()
{
}

void ArgumentPasser::badargs() const
{
    ConsoleOutput::print("Type -h or --help for help.", ConsoleOutput::QUESTION);
}
void ArgumentPasser::multiple_encrypt_decrypt_options() const
{
    ConsoleOutput::print("Option -c or -d specified multiple times or both. Quit.", ConsoleOutput::ERROR);
}
bool ArgumentPasser::consumeArgument(string&& arg, Settings* s, const cmdOptions* options) const
{
    if (arg.length() < 1)
        return false;

    if (arg[0] == '-')
    {
        if (arg.length() >= 3 && arg[0] == arg[1])
        {//-- option
            if(options->matches(arg, cmdOptions::OPTION_NAME::PRINT_ALL_OPTIONS))
                s->printAllOptions = true;            
            else if (options->matches(arg, cmdOptions::OPTION_NAME::CHANGE_NAMES_Y))
            {
                if(s->ChangeFileNames_option_found)
                {
                    ConsoleOutput::print("Option --change-names can not be specified multiple times.", ConsoleOutput::ERROR);
                    return false;
                }
                s->ChangeFileNames_option_found = true;
                s->ChangeFileNames = true;
            }
            else if (options->matches(arg, cmdOptions::OPTION_NAME::CHANGE_NAMES_N))
            {
                if(s->ChangeFileNames_option_found)
                {
                    ConsoleOutput::print("Option --change-names can not be specified multiple times.", ConsoleOutput::ERROR);
                    return false;
                }
                s->ChangeFileNames_option_found = true;
                s->ChangeFileNames = false;
            }
            else if (options->matches(arg, cmdOptions::OPTION_NAME::CHECK))
                s->CheckForUnEncryptedFiles = true;
            else if(options->matches(arg, cmdOptions::OPTION_NAME::ENCRYPT))
            {
                if(!found_enc_dec_option(true, s))
                    return false;
            }
            else if(options->matches(arg, cmdOptions::OPTION_NAME::DECRYPT))
            {
                if(!found_enc_dec_option(false, s))
                    return false;
            }
            else if(options->matches(arg, cmdOptions::OPTION_NAME::FIND))
                s->find_encrypted = true;
            else if(options->matches(arg, cmdOptions::OPTION_NAME::FORCE))
                s->EncryptAllExtensions = true;
            else if (options->matches(arg, cmdOptions::OPTION_NAME::HELP))
                s->printHelp = true;
            else if (options->matches(arg, cmdOptions::OPTION_NAME::NO_DELETE))
                s->no_delete = true;
            else if(options->matches(arg, cmdOptions::OPTION_NAME::NO_THREADS))
                s->no_threads = true;
            else if (options->matches(arg, cmdOptions::OPTION_NAME::RECURSIVE))
                s->recursive = true;
            else if (options->matches(arg, cmdOptions::OPTION_NAME::TESTS))
                s->runTests = true;
            else if (options->matches(arg, cmdOptions::OPTION_NAME::VERSION))
                s->printVersion = true;
            else if (options->matches(arg, cmdOptions::OPTION_NAME::VERBOSE))
                s->verbose = true;
            else
            {
                print_unrecognized_option(arg);
                return false;
            }
        }
        else if(arg.length() >= 2 && arg[0] == '-')
        {//- option
            return consumeSingleArgument(arg, s, options);
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

bool ArgumentPasser::consumeSingleArgument(const string& arg, Settings* s, const cmdOptions* options) const
{
    for (size_t i = 1; i < arg.length(); i++)
    {
        if (arg[i] == 'c')
        {
            if(!found_enc_dec_option(true, s))
                return false;
        }
        else if (arg[i] == 'd')
        {
           if(!found_enc_dec_option(false, s))
                return false;
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
bool ArgumentPasser::found_enc_dec_option(bool encrypt, Settings* s) const
{
     if (encrypt)
        {
            if(s->encrypt_or_decrypt_option_found)
            {
                multiple_encrypt_decrypt_options();
                s->SetExitProgram_And_ExitCode(1);
                return false;
            }
            s->encrypt = true;
            s->encrypt_or_decrypt_option_found = true;
            return true;
        }
        else
        {
            if(s->encrypt_or_decrypt_option_found)
            {
                multiple_encrypt_decrypt_options();
                s->SetExitProgram_And_ExitCode(1);
                return false;
            }
            s->encrypt = false;
            s->encrypt_or_decrypt_option_found = true;
            return true;
        }
     return true;
}
bool ArgumentPasser::ParseArguments(char** args, int argc, Settings* s, const cmdOptions* options)
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
                
        if (!consumeArgument(move(arg), s, options))
            return false;
    }    
    return true;
}

inline void ArgumentPasser::print_unrecognized_option(const string& opt) const
{
    ConsoleOutput::print("Unrecognized option '" + opt + "'. Quit.", ConsoleOutput::ERROR);
}