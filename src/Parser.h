#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <stack>
#include <set>
#include "Token.h"
#include "Node.h"
using namespace std;

class Parser {
public:
    Parser();
    void setParseTable(map<string, map<string, vector<string> > > table);
    void setTerminals(set<string> t);
    void setNonTerminals(set<string> nt);
    void generateParseTree(vector<Token> allTokens);
    void exportTree(string fileName = "tree.txt");

    vector<Node> getParseTree();
    void printTree();

private:
    map<string, map<string, vector<string> > > parseTable;
    set<string> terminals;
    set<string> nonTerminals;
    vector<Node> tree;

    bool isTerminal(string symbol);
    bool isNonTerminal(string symbol);
    bool entryExistInParseTable(string key1, string key2);
};
#endif
