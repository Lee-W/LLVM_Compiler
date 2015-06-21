#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include <map>
#include <vector>
#include <stack>
#include "Symbol.h"
#include "Node.h"

class SemanticAnalyzer {
public:
    void analysis(vector<Node> parseTree);
    void printSymbolTable();
private:
    map<int, vector<Symbol>> symbolTable;

    void readNextLayer(vector<Node>::iterator& it, string& symbol);
    void tableInsert(Symbol s);
    bool symbolExistInSameScope(Symbol s);
};

#endif