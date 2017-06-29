#include "ConsoleOutput.h"
#include <iostream>

#include "../Colors/Colors.h"

mutex ConsoleOutput::print_mtx;

ConsoleOutput::ConsoleOutput()
{
}

ConsoleOutput::~ConsoleOutput()
{
}

void ConsoleOutput::print(const string& what, MESSAGE_TYPE m, string CustomWord)
{
    string sign;
    ostream* os = &cout;

    string color = "";

    //set color
    if (m == MESSAGE_TYPE::OK)
        color = Colors::GREEN;
    else if (m == MESSAGE_TYPE::ERROR || m == MESSAGE_TYPE::CODE_ERROR || m == MESSAGE_TYPE::FUNCTION_ERROR)
        color = Colors::RED;
    else if (m == MESSAGE_TYPE::QUESTION || m == MESSAGE_TYPE::WARNING)
        color = Colors::YELLOW;

    //set text and OS

    if (m == MESSAGE_TYPE::OK)
        sign = "+";
    else if (m == MESSAGE_TYPE::ERROR)
    {
        sign = "-";
        os = &cerr;
    }
    else if (m == MESSAGE_TYPE::QUESTION)
        sign = "?";
    else if (m == MESSAGE_TYPE::CODE_ERROR)
    {
        sign = "IN CODE ERROR";
        os = &cerr;
    }
    else if (m == MESSAGE_TYPE::FUNCTION_ERROR)
    {
        sign = "BUILT IN FUNCTION ERROR";
        os = &cerr;
    }
    else if (m == MESSAGE_TYPE::WARNING)
    {
        sign = "!";
        os = &cerr;
    }
    if (CustomWord != "")
        sign = CustomWord;

    print_mtx.lock();
    *os << "[" << color << sign << Colors::RESET << "] " << what << endl;
    print_mtx.unlock();
}