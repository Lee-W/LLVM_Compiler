#ifndef LEXER_H
#define LEXER_H

#include <fstream>
#include <iomanip>
#include <set>
#include <vector>
#include "Token.h"

class Lexer {
public:
    Lexer();
    void feed(string line);
    void clearResult();
    vector<vector<Token> > getResult();

private:
    const static set<string> KEY_WORDS;
    const static set<string> OPERATORS;
    const static set<string> SPECIAL_SYMBOLS;
    const static set<string> COMMENT;
    const static string CHAR_IDENTIFIER;
    const static string ESCAPE_SYSBOL;
    const static string DOT_POINT;

    vector<vector<Token> > result;

    // handle Keyword, number and identifier
    Token handleWord(string st);

    bool isIdentifierFirstElement(string ch);
    bool isIdentifierElement(string ch);

    bool isUpperAlphabet(string ch);
    bool isUpperAlphabet(char c);

    bool isLowerAlphabet(string ch);
    bool isLowerAlphabet(char c);

    bool isAlphabet(string ch);
    bool isAlphabet(char c);

    bool isDigit(string ch);
    bool isDigit(char c);

    bool isEmptyChacter(string ch);
    bool isEmptyChacter(char c);

    template <class T>
    bool contain(set<T> container, T content);
};

#endif
