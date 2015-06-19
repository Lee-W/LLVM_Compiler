#include "SemanticAnalyzer.h"

void SemanticAnalyzer::analysis(vector<Node> parseTree)
{
    int scope = 0;
    int scopeCounter = 0;

    bool isDecl = false;
    bool isParDecl = false;
    bool isFunDecl = false;

    string symbol;
    string type;
    bool isArray = false;

    stack<Symbol> symbolStack;
    stack<Symbol> paraStack;

    string curSymbol;
    for (auto it = parseTree.begin(); it != parseTree.end(); ++it) {
        curSymbol = it->symbol;

        if (curSymbol == "{") {
            scopeCounter++;
            scope = scopeCounter;
            symbolStack.push(Symbol(scopeCounter, "{", type, false, false));
            while (!paraStack.empty()) {
                symbolStack.push(paraStack.top());
                paraStack.pop();
            }
        } else if (curSymbol == "}") {
            tableInsert(symbolStack);
            scope = symbolStack.top().scope;
        } else if (curSymbol == "DeclList'" || curSymbol == "VarDecl") {
            isDecl = true;
        } else if (curSymbol == "Type" && (isDecl || isParDecl)) {
            readNextLayer(it, curSymbol);
            type = curSymbol;
        } else if (curSymbol == "id") {
            readNextLayer(it, curSymbol);
            symbol = curSymbol;
        } else if (curSymbol == "VarDecl'") {
            readNextLayer(it, curSymbol);
            if (curSymbol == "[")
                isArray = true;

            symbolStack.push(Symbol(scope, symbol, type, isArray, isFunDecl));
            isDecl = false;
            isFunDecl = false;
            isArray = false;
        } else if (curSymbol == "FunDecl") {
            isFunDecl = true;

            symbolStack.push(Symbol(scope, symbol, type, isArray, isFunDecl));
            isFunDecl = false;
        } else if (curSymbol == "ParamDecl") { // && isFunDecl) {
            isParDecl = true;
        } else if (curSymbol == "ParamDecl'" && isParDecl) {
            readNextLayer(it, curSymbol);
            if (curSymbol == "[")
                isArray = true;
        } else if (curSymbol == "ParamDeclListTail'") {

            // symbolStack.push(Symbol(scope+1, symbol, type, isArray, isFunDecl));
            paraStack.push(Symbol(scope+1, symbol, type, isArray, isFunDecl));
            isParDecl = false;
            isArray = false;
        }
    }

    while (!symbolStack.empty()) {
        Symbol s = symbolStack.top();
        symbolStack.pop();

        if (s.symbol != "{") {
            symbolTable.insert(make_pair(s.scope, Symbol(s.scope, s.symbol, s.type, s.isArray, s.isFunction)));
            // cout << s.scope << "\t" << s.symbol << "\t" << s.type << "\t" << s.isArray << "\t" << s.isFunction << endl;
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
    int prevScope = 0;
    for (auto entry : symbolTable) {
        Symbol s = entry.second;
        if (s.scope != prevScope) {
            cout << endl;
        }
        prevScope = s.scope;
        cout << s.scope << "\t" << s.symbol << "\t" << s.type << "\t" << s.isArray << "\t" << s.isFunction << endl;
    }
}

void SemanticAnalyzer::tableInsert(stack<Symbol>& s) {
    Symbol sym;
    while (sym.symbol != "{" && !s.empty()) {
        sym = s.top();
        s.pop();

        if (sym.symbol != "{") {
            symbolTable.insert(make_pair(sym.scope, Symbol(sym.scope, sym.symbol, sym.type, sym.isArray, sym.isFunction)));
            // cout << sym.scope << "\t" << sym.symbol << "\t" << sym.type << "\t" << sym.isArray << "\t" << sym.isFunction << endl;
        }

    }
}
