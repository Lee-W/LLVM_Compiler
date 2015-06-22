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
            Symbol topSymbol;
            while (topSymbol.symbol != "{" && !symbolStack.empty()) {
                topSymbol = symbolStack.top();
                symbolStack.pop();
            }
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
            tableInsert(Symbol(scope, symbol, type, isArray, isFunDecl));

            isDecl = false;
            isFunDecl = false;
            isArray = false;
        } else if (curSymbol == "FunDecl") {
            isFunDecl = true;

            symbolStack.push(Symbol(scope, symbol, type, isArray, isFunDecl));
            tableInsert(Symbol(scope, symbol, type, isArray, isFunDecl));

            isFunDecl = false;
        } else if (curSymbol == "ParamDecl") { // && isFunDecl) {
            isParDecl = true;
        } else if (curSymbol == "ParamDecl'" && isParDecl) {
            readNextLayer(it, curSymbol);
            if (curSymbol == "[")
                isArray = true;
        } else if (curSymbol == "ParamDeclListTail'") {

            paraStack.push(Symbol(scope+1, symbol, type, isArray, isFunDecl));
            tableInsert(Symbol(scope+1, symbol, type, isArray, isFunDecl));
            isParDecl = false;
            isArray = false;
        }
    }

    while (!symbolStack.empty()) {
        Symbol s = symbolStack.top();
        symbolStack.pop();
    }
}

void SemanticAnalyzer::readNextLayer(vector<Node>::iterator& it, string& symbol)
{
    ++it;
    symbol = it->symbol;
}

void SemanticAnalyzer::printSymbolTable()
{
    for (auto entry: symbolTable) {
        for (auto s : entry.second)
            cout << s.scope << "\t" << s.symbol << "\t" << s.type << "\t" << s.isArray << "\t" << s.isFunction << endl;
        cout << endl;
    }
}

map<int, vector<Symbol> > SemanticAnalyzer::getSymbolTable()
{
	return symbolTable;
}

void SemanticAnalyzer::tableInsert(Symbol s)
{
    if (!symbolExistInSameScope(s))
        symbolTable[s.scope].push_back(s);
    else {
        cout << "Already declaraed" << endl;
    }
}

bool SemanticAnalyzer::symbolExistInSameScope(Symbol s)
{
    vector<Symbol> symbolList = symbolTable[s.scope];
    for (auto sym : symbolList)
        if (s.symbol == sym.symbol)
            return true;
    return false;
}

void SemanticAnalyzer::exportSymbolTable(string fileName)
{
    ofstream outputFileStream(fileName);
    if (outputFileStream.is_open()) {
        for (auto entry: symbolTable) {
            for (auto s : entry.second)
                outputFileStream << s.scope << "\t" << s.symbol << "\t" << s.type << "\t" << s.isArray << "\t" << s.isFunction << endl;
            outputFileStream << endl;
        }
        outputFileStream.close();
    }
    else {
        cout << "Cannot open file " << fileName << endl;
    }
}
