#include "Lexer.h"

Lexer::Lexer()
{
}

void Lexer::feed(string line)
{
    vector<Token> tokens;
    string curToken;

    for (int i = 0; i < line.size(); i++) {
        string ch = string(1, line[i]);

        if (isEmptyChacter(ch)) {
            if (curToken.size() > 0) {
                tokens.push_back(handleWord(curToken));
                curToken = string();
            }
            continue;
        }

        if (isIdentifierElement(ch) || ch == DOT_POINT) {
            // handle identifier, number and keyword
            curToken += ch;
        }
        else {
            // encoutering chatacter other than identiier, number and Keyword
            // handle current token first
            if (curToken.size() > 0) {
                tokens.push_back(handleWord(curToken));
                curToken = string();
            }

            if (contain(SPECIAL_SYMBOLS, ch)) {
                // handle sepcial symbol
                tokens.push_back(Token("Special", ch));
            }
            else if (contain(OPERATORS, ch)) {
                // handle operators and COMMENT
                string next_token = ch + string(1, line[i + 1]);

                if (contain(COMMENT, next_token))
                    break;
                else if (contain(OPERATORS, next_token)) {
                    i++;
                    tokens.push_back(Token("Operator", next_token));
                }
                else
                    tokens.push_back(Token("Operator", ch));
            }
            else if (ch == CHAR_IDENTIFIER) {
                string next_ch = string(1, line[i + 1]);
                string next_token = ch + next_ch + string(1, line[i + 2]);

                if (next_ch == ESCAPE_SYSBOL) {
                    next_token += string(1, line[i + 3]);
                    i++;
                }
                i = i + 2;
                tokens.push_back(Token("Char", next_token));
            }
            else {
                string next_token = ch + string(1, line[i + 1]);
                if (contain(OPERATORS, next_token)) {
                    i++;
                    tokens.push_back(Token("Operator", next_token));
                }
                else
                    tokens.push_back(Token("Not in spec", ch));
            }
        }
    }
    if (curToken.size() > 0) {
        tokens.push_back(handleWord(curToken));
        curToken = string();
    }
    result.push_back(tokens);
}

void Lexer::clearResult()
{
    result.clear();
}

vector<vector<Token> > Lexer::getResult()
{
    return result;
}

// handle Keyword, number and identifier
Token Lexer::handleWord(string st)
{
    string firstCh = string(1, st.front());

    if (isDigit(firstCh)) { /* number */
        bool isFloat = false;

        for (auto c : st) {
            string ch = string(1, c);
            if (!isDigit(ch) && ch != DOT_POINT)  // identifier start with digit
                return Token("Error", st);
            else if (ch == DOT_POINT) {
                if (!isFloat)
                    isFloat = true;
                else  // exist more than one dot
                    return Token("Error", st);
            }
        }

        if (isFloat && !isDigit(st.back()))  // number ended with dot. e.g. 123.
            return Token("Error", st);

        return Token("Number", st);
    }
    else if (contain(KEY_WORDS, st)) { /* Keyword */
        return Token("Keyword", st);
    }
    else { /* Identifier */
        if (!isIdentifierFirstElement(
                firstCh))  // identifier that not start with proper element
            return Token("Error", st);

        for (auto c : st) {
            string ch = string(1, c);
            if (!isIdentifierElement(ch))  // exist not identifier element
                return Token("Error", st);
        }
        return Token("Identifier", st);
    }
}

bool Lexer::isIdentifierFirstElement(string ch)
{
    return isAlphabet(ch) || ch == "_";
}

bool Lexer::isIdentifierElement(string ch)
{
    return isAlphabet(ch) || isDigit(ch) || ch == "_";
}

bool Lexer::isUpperAlphabet(string ch)
{
    char c = ch.front();
    return (isUpperAlphabet(c));
}

bool Lexer::isUpperAlphabet(char c)
{
    return c >= 'A' && c <= 'Z';
}

bool Lexer::isLowerAlphabet(string ch)
{
    char c = ch.front();
    return isLowerAlphabet(c);
}

bool Lexer::isLowerAlphabet(char c)
{
    return c >= 'a' && c <= 'z';
}

bool Lexer::isAlphabet(string ch)
{
    char c = ch.front();
    return isLowerAlphabet(c) || isUpperAlphabet(c);
}

bool Lexer::isAlphabet(char c)
{
    return isLowerAlphabet(c) || isUpperAlphabet(c);
}

bool Lexer::isDigit(string ch)
{
    char c = ch.front();
    return isDigit(c);
}

bool Lexer::isDigit(char c)
{
    return c >= '0' && c <= '9';
}

bool Lexer::isEmptyChacter(string ch)
{
    char c = ch.front();
    return isEmptyChacter(c);
}

bool Lexer::isEmptyChacter(char c)
{
    return c == ' ' || c == '\n' || c == '\t';
}

template <class T>
bool Lexer::contain(set<T> container, T content)
{
    return (container.find(content) != container.end());
}

const set<string> Lexer::KEY_WORDS{"int",    "char", "float", "double",
                                   "return", "if",   "else",  "while",
                                   "break",  "for",  "print"};
const set<string> Lexer::OPERATORS{"=",  "!", "+", "-",  "*",  "/",  "==",
                                   "!=", "<", ">", "<=", ">=", "&&", "||"};
const set<string> Lexer::SPECIAL_SYMBOLS{"[", "]", "(", ")",
                                         "{", "}", ";", ","};
const set<string> Lexer::COMMENT{"//"};
const string Lexer::CHAR_IDENTIFIER = "'";
const string Lexer::ESCAPE_SYSBOL = "\\";
const string Lexer::DOT_POINT = ".";
