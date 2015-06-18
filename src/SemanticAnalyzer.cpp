#include "SemanticAnalyzer.h"

void SemanticAnalyzer::analysis(vector<Node> parseTree)
{
    int scope = 0;
    int level = 0;
    int scopeCounter = 0;

    bool isDecl = false;
    bool isParDecl = false;
    bool isFunDecl = false;

    string symbol;
    string type;
    bool isArray = false;

    string curSymbol;
    for (auto it = parseTree.begin(); it != parseTree.end(); ++it) {
        curSymbol = it->symbol;

        if (curSymbol == "DeclList'") {
            isDecl = true;
        } else if (curSymbol == "Type" && (isDecl || isParDecl)) {
            readNextLayer(it, symbol);
            type = curSymbol;
        } else if (curSymbol == "id") {
            readNextLayer(it, symbol);
            symbol = curSymbol;
        } else if (curSymbol == "VarDecl'") {
            isDecl = true;

            readNextLayer(it, symbol);
            if (curSymbol == "[")
                isArray = true;

            symbolTable.insert(make_pair(scope, Symbol(scope, symbol, type, isArray, isFunDecl)));
            isDecl = false;
            isFunDecl = false;
            isArray = false;
        } else if (curSymbol == "FunDecl") {
            isFunDecl = true;
            symbolTable.insert(make_pair(scope, Symbol(scope, symbol, type, isArray, isFunDecl)));
            isFunDecl = false;
        } else if (curSymbol == "ParamDecl" && isFunDecl) {
            isParDecl = true;
        } else if (curSymbol == "ParamDecl'" && isParDecl) {
            if (curSymbol == "[")
                isArray = true;
            else if (curSymbol == "epsilon") {
                symbolTable.insert(make_pair(scope, Symbol(scope, symbol, type, isArray, isFunDecl)));

                isParDecl = false;
                isArray = false;
            }
        } else if (curSymbol == "ParamDeclListTail'" && isParDecl) {
            isParDecl = false;
            symbolTable.insert(make_pair(scope, Symbol(scope, symbol, type, isArray, isFunDecl)));
        } else if (curSymbol == "{") {
            isFunDecl = false;
            scope = ++scopeCounter;
        }
    }
}

void SemanticAnalyzer::readNextLayer(vector<Node>::iterator& it, string& symbol)
{
    ++it;
    symbol = it->symbol;
}

void SemanticAnalyzer::printSymbolTable()
{
    for (auto entry : symbolTable) {
        Symbol s = entry.second;
            cout << s.scope << "\t" << s.symbol << "\t" << s.type << "\t" << s.isArray << "\t" << s.isFunction << endl;
    }
}
