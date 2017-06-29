#ifndef CORE_H
#define CORE_H

#include <unordered_set>
#include <string>

#include "Settings/Settings.h"

using namespace std;
/**
 * @brief entry point of app
 */
struct Core
{
    
    Core();
    Core(const Core& s) = delete;
    ~Core();

private:
    inline void FreeAllocatedArguments();
    
public:
    /**
     * @brief Gets password from user
     * @param pass Variable to fill with the password
     * @return Whether password was valid and all ok
     */
    inline bool GetEncryptionKey(string& pass) const;
private:
    
    
    inline string getfullexepath(char* argv_path) const;
    
    void HideStdinKeystrokes() const;
   
public:
    Core& operator=(const Core& o) = delete;
    
private:
    /**
     * @brief prints help for this application
     * @return value 0
     */
    inline int printHelp() const;
    inline void printOptionHelp(string&& option, string&& decription) const;
    /**
     * @brief prints version for this application
     * @return value 0
     */
    inline int printVersion() const;

public:
    int run(int argc, char** argv);
    
private:    
    /**
     * @brief Used for running tests
     * @return 
     */
    inline int RunTests(string&& runningdirectory, const Settings& s) const;
    inline void ShowStdinKeystrokes() const;
    
    /**
     * @brief Used for debugging only. Modifies the command line arguments to desired ones
     * @param argc
     * @param argv
     * @param args
     */
    inline void SimulateArgs(int& argc, char**& argv, initializer_list<string> args);
    
    int argc_allocated;
    char** argv_allocated;
    bool SimulatedArguments;
    static char VERSION[]; /**< Version of this app */
    const string APP_NAME = "cryptor";
};

#endif /* CORE_H */

