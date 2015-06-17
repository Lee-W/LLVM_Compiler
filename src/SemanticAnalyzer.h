#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include <map>
#include <vector>
#include "Symbol.h"
#include "Node.h"

class SemanticAnalyzer {
public:
    void analysis(vector<Node> parseTree);
private:
    multimap<int, Symbol> symbolTable;

};

#endif
