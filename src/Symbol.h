#ifndef SYMBOL_H
#define SYMBOL_H

#include <iostream>
using namespace std;


struct Symbol {
    Symbol() {}

    Symbol(int s, string sy, string t)
    {
        scope = s;
        symbol = sy;
        type = t;
    }

    Symbol(int s, string sy, string t, bool isA, bool isF)
    {
        scope = s;
        symbol = sy;
        type = t;
        isArray = isA;
        isFunction = isF;
    }

    int scope;
    string symbol;
    string type;
    bool isArray;
    bool isFunction;
};

#endif
