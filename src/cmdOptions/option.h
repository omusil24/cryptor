#pragma once

#include <unordered_set>

#include "cmdOptions.h"


using namespace std;
struct option
{
    option(cmdOptions::OPTION_NAME n);
    ~option();
    
    void addOption(const string& text);
    void addOption(string&& text);
    cmdOptions::OPTION_NAME getName() const;
    string getOptionTextString() const;
    bool hasOption(const string& text) const;
    
private:
    cmdOptions::OPTION_NAME name;
    unordered_set<string> option_texts;
};