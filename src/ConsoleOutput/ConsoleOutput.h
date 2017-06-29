#ifndef CONSOLEOUTPUT_H
#define CONSOLEOUTPUT_H

#include <mutex>
#include <string>

using namespace std;


struct ConsoleOutput
{
    enum MESSAGE_TYPE
    {
        CODE_ERROR, /**< When there is error in the written code or logic error */
        OK,
        ERROR,
        FUNCTION_ERROR,
        QUESTION,
        WARNING
    };
    ConsoleOutput();
    ConsoleOutput(const ConsoleOutput& a) =delete;
    ~ConsoleOutput();
    
    ConsoleOutput& operator=(const ConsoleOutput& o) = delete;
    
    static void print(const string& what, MESSAGE_TYPE m = MESSAGE_TYPE::OK, string CustomWord = "");
    
private:
    static mutex print_mtx;
};


#endif /* CONSOLEOUTPUT_H */

