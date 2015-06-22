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
    map<int, vector<Symbol>> getSymbolTable();
    void exportSymbolTable(string fileName="SymbolTable.txt");

    void printSymbolTable();
private:
    map<int, vector<Symbol>> symbolTable;

    void readNextLayer(vector<Node>::iterator& it, string& symbol);
    void tableInsert(Symbol s);
    bool symbolExistInSameScope(Symbol s);
};

#endif
