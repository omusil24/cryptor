#ifndef COLORS_H
#define COLORS_H

#include <string>

using namespace std;

struct Colors {
    Colors();
    Colors(const Colors& c) = delete;
    ~Colors();

    static const string BOLD;
    static const string CYAN;
    static const string GREEN;
    static const string RED;
    static const string RESET;
    static const string UNDERLINE;
    static const string YELLOW;
};

#endif /* COLORS_H */

