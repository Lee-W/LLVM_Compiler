#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include <map>
#include <vector>
#include "Symbol.h"
#include "Node.h"

class SemanticAnalyzer {
public:
    void analysis(vector<Node> parseTree);
    void printSymbolTable();
private:
    multimap<int, Symbol> symbolTable;

    void readNextLayer(vector<Node>::iterator& it, string& symbol);
};

#endif
