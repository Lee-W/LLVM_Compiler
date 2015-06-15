#ifndef TOKEN_H
#define TOKEN_H

#include <iostream>
using namespace std;

struct Token {
    Token() {}

    Token(string cate, string cont)
    {
        category = cate;
        content = cont;
    }

    string category;
    string content;
};

#endif
