#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <iostream>
#include <fstream>
#include <sstream>
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
    void exportLlvmCode(string fileName="output.ll");

private:
    const static map<string, int> OP_PRIORITY;

    vector<string> llvmCode;
    map<int, vector<Symbol> > symbolTable;
    int instruction = 1;  //%1 %2 %3 %4 %5......

    vector<string> declaration(vector<Node>::iterator it);
    vector<string> varDecl(vector<Node>::iterator it);
    vector<string> expr(vector<Node>::iterator it);
    vector<string> statement(vector<Node>::iterator it);
    vector<string> ifElse(vector<Node>::iterator it);
    vector<string> printID(vector<Node>::iterator it);
    vector<string> whileStatement(vector<Node>::iterator it);
    const char* typeCast(string type);
    Symbol findType(vector<Node>::iterator it);
    Symbol findSymbol(string symbol);

    vector<string> handleExpr(vector<Symbol> expr);
    vector<Symbol> infixExprToPostfix(vector<string> expr);
    bool isOperator(string symbol);

    void appendVectors(vector<string>& v1, vector<string> v2);
};


#endif


