#include "cmdOptions.h"
#include "option.h"
#include "../ConsoleOutput/ConsoleOutput.h"

cmdOptions::cmdOptions()
{
    initialize();
}

cmdOptions::~cmdOptions()
{
    for (auto i = opts.begin(); i != opts.end(); i++)
        delete i->second;
    opts.clear();
}

void cmdOptions::addOption(OPTION_NAME n, initializer_list<string>&& s)
{
    option* o = new option(n);
    if (!opts.insert(make_pair(n, o)).second)
    {
        delete o;
        return;
    }
    for (auto i = s.begin(); i != s.end(); i++)
        o->addOption(move(*i));
}

string cmdOptions::getAllOptionTextString()const
{
    string res = "";
    for (auto i = opts.begin(); i != opts.end(); i++)
    {
        if (i == opts.begin())
            res += i->second->getOptionTextString();
        else
            res += " " + i->second->getOptionTextString();
    }
    return res;
}

string cmdOptions::getOptionText(OPTION_NAME opt)const
{
    auto res = opts.find(opt);
    if (res == opts.end())
    {
        ConsoleOutput::print("No specification for option", ConsoleOutput::CODE_ERROR);
        return "[?]";
    }
    return res->second->getOptionTextString();
}

void cmdOptions::initialize()
{
    addOption(OPTION_NAME::ENCRYPT,{"-c", "--crypt", "--encrypt"});
    addOption(OPTION_NAME::DECRYPT,{"-d", "--decrypt"});
    addOption(OPTION_NAME::RECURSIVE,{"-r", "--recursive"});
    addOption(OPTION_NAME::CHANGE_NAMES_N,{"--change-names=no", "--change-names=n"});
    addOption(OPTION_NAME::CHANGE_NAMES_Y,{"--change-names=yes", "--change-names=y"});
    addOption(OPTION_NAME::CHECK,{"--check"});
    addOption(OPTION_NAME::FIND,{"--find"});
    addOption(OPTION_NAME::FORCE,{"--force"});
    addOption(OPTION_NAME::HELP,{"-h", "--help"});
    addOption(OPTION_NAME::NO_DELETE,{"--no-del", "--no-delete"});
    addOption(OPTION_NAME::NO_THREADS,{"--no-threads"});
    addOption(OPTION_NAME::PRINT_ALL_OPTIONS,{"--print-all-options"});
    addOption(OPTION_NAME::TESTS,{"--test", "--tests"});
    addOption(OPTION_NAME::VERBOSE,{"-v", "--verbose"});
    addOption(OPTION_NAME::VERSION,{"--ver", "--version"});
}

bool cmdOptions::matches(const string& s, OPTION_NAME opt)const
{
    auto res = opts.find(opt);
    if (res == opts.end())
    {
        ConsoleOutput::print("No specification for option", ConsoleOutput::CODE_ERROR);
        return false;
    }
    return res->second->hasOption(s);
}