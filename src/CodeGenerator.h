#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H


#include <iostream>
#include <stdio.h>
#include <vector>
#include <map>
#include <stack>
#include "Node.h"
#include "Symbol.h"
using namespace std;

class CodeGenerator {
public:
    void setSymbolTable(map<int, vector<Symbol> > st);
    void codeGeneration(vector<Node> parseTree);
    void testFunctions();
private:
    const static map<string, int> OP_PRIORITY;
    map<int, vector<Symbol> > symbolTable;
    FILE* llFile;
    int instruction = 1;  //%1 %2 %3 %4 %5......

    void declaration(vector<Node>::iterator it);
    void varDecl(vector<Node>::iterator it);
    void expr(vector<Node>::iterator it);
    void statement(vector<Node>::iterator it);
    void printID(vector<Node>::iterator it);
    const char* typeCast(string type);
    Symbol findType(vector<Node>::iterator it);
    Symbol findSymbol(string symbol);

    void handleExpr(vector<Symbol> expr);
    vector<Symbol> infixExprToPostfix(vector<string> expr);
    bool isOperator(string symbol);
};


#endif


