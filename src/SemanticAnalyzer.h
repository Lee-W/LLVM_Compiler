#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include <map>
#include <vector>
#include <stack>
#include <fstream>
#include "Symbol.h"
#include "Node.h"

class SemanticAnalyzer {
public:
    void analysis(vector<Node> parseTree);
    map<int, vector<Symbol> > getSymbolTable();
    void exportSymbolTable(string fileName = "SymbolTable.txt");

    void printSymbolTable();

private:
    const static map<string, int> TYPE_PRIORITY;
    const static map<string, int> OP_PRIORITY;
    map<int, vector<Symbol> > symbolTable;

    void readNextLayer(vector<Node>::iterator& it, string& symbol,
                       int readNum = 1);
    void tableInsert(Symbol s);
    bool symbolExistInSameScope(Symbol s);

    void checkType(vector<string> expr, int scope);
    vector<Symbol> infixExprToPostfix(vector<string> expr, int scope);
    void printTypeWarning(int scope, Symbol s1, Symbol s2);
    string getType(int scope, string symbol);
    string typeCasting(Symbol s1, Symbol s2);
    Symbol accessSymbolTable(int scope, string symbol);
    bool isID(string symbol);
    bool isDouble(string symbol);

    bool isOperator(string symbol);
};

#endif
