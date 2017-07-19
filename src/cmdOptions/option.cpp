#include "option.h"

option::option(cmdOptions::OPTION_NAME n) : name(n)
{
    
}
option::~option()
{
    
}
void option::addOption(const string& text)
{
    option_texts.insert(text);
}
void option::addOption(string&& text)
{
    option_texts.insert(move(text));
}
cmdOptions::OPTION_NAME option::getName() const
{
    return name;
}
string option::getOptionTextString() const
{
    string res = "";
    for(auto i = option_texts.begin(); i != option_texts.end(); i++)
    {
        if( i == option_texts.begin())
            res += *i;
        else
            res += " " + *i;
    }
    return res;
}
bool option::hasOption(const string& text) const
{
    return (option_texts.find(text) != option_texts.end());
}