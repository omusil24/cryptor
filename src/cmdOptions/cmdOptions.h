#pragma once

#include <string>
#include <unordered_map>

class option;

using namespace std;

/**
 * @brief Used for holding all possible cmd options, and used for determining if they match
 */
struct cmdOptions {

    enum class OPTION_NAME {
        CHANGE_NAMES_Y,
        CHANGE_NAMES_N,
        CHECK,
        DECRYPT,
        ENCRYPT,
        FIND,
        FORCE,
        HELP,
        NO_DELETE,
        NO_THREADS,
        RECURSIVE,
        PRINT_ALL_OPTIONS,
        TESTS,
        VERBOSE,
        VERSION
    };

    cmdOptions();
    ~cmdOptions();

private:
    void addOption(OPTION_NAME n, initializer_list<string>&& s);

public:
    string getAllOptionTextString() const;
    string getOptionText(OPTION_NAME) const;
private:
    void initialize();
    
public:
    /**
     * 
     * @param s
     * @return If option s matches option opt 
     */
    bool matches(const string& s, OPTION_NAME opt) const;

    unordered_map<OPTION_NAME, option*> opts;
};