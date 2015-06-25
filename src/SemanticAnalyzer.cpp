#include "SemanticAnalyzer.h"

void SemanticAnalyzer::analysis(vector<Node> parseTree)
{
    int scope = 0;
    int scopeCounter = 0;

    bool isDecl = false;
    bool isFunDecl = false;
    bool isExpr = false;

    string symbol;
    string type;
    bool isArray = false;

    stack<Symbol> symbolStack;
    stack<Symbol> paraStack;

    bool isRightHandSide = false;
    string leftHandSide;
    vector<string> rightHandSide;

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
        }
        else if (curSymbol == "}") {
            Symbol topSymbol;
            while (topSymbol.symbol != "{" && !symbolStack.empty()) {
                topSymbol = symbolStack.top();
                symbolStack.pop();
            }
            scope = symbolStack.top().scope;
        }
        else if (curSymbol == "DeclList'" || curSymbol == "VarDecl") {
            isDecl = true;
        }
        else if (curSymbol == "Type" && (isDecl)) {
            readNextLayer(it, curSymbol);
            type = curSymbol;
        }
        else if (curSymbol == "id") {
            readNextLayer(it, curSymbol);
            symbol = curSymbol;
        }
        else if (curSymbol == "VarDecl'") {
            readNextLayer(it, curSymbol);
            if (curSymbol == "[")
                isArray = true;

            symbolStack.push(Symbol(scope, symbol, type, isArray, isFunDecl));
            tableInsert(Symbol(scope, symbol, type, isArray, isFunDecl));

            isDecl = false;
            isFunDecl = false;
            isArray = false;
        }
        else if (curSymbol == "FunDecl") {
            isFunDecl = true;

            symbolStack.push(Symbol(scope, symbol, type, isArray, isFunDecl));
            tableInsert(Symbol(scope, symbol, type, isArray, isFunDecl));
            isFunDecl = false;

            readNextLayer(it, curSymbol);
            while (curSymbol != ")") {
                if (curSymbol == "ParamDecl") {
                    readNextLayer(it, curSymbol, 2);
                    type = curSymbol;

                    readNextLayer(it, curSymbol, 2);
                    symbol = curSymbol;
                }
                else if (curSymbol == "ParamDecl'") {
                    readNextLayer(it, curSymbol);
                    if (curSymbol == "[") {
                        isArray = true;
                        readNextLayer(it, curSymbol);
                    }
                }
                else if (curSymbol == "ParamDeclListTail'") {
                    paraStack.push(
                        Symbol(scope + 1, symbol, type, isArray, isFunDecl));
                    tableInsert(
                        Symbol(scope + 1, symbol, type, isArray, isFunDecl));

                    isArray = false;
                }
                readNextLayer(it, curSymbol);
            }
        }
        else if (curSymbol == "Expr") {
            int treeLayer = it->layer;

            readNextLayer(it, curSymbol);
            while (it->layer != treeLayer) {
                if (curSymbol == "ExprIdTail" || curSymbol == "ExprArrayTail") {
                    readNextLayer(it, curSymbol);

                    if (curSymbol == "=") {
                        isRightHandSide = true;
                        rightHandSide.clear();
                    }
                    else if (curSymbol == "[") {
                        while (curSymbol != "]")
                            readNextLayer(it, curSymbol);
                    }
                    else if (curSymbol == "(") {
                        while (curSymbol != ")")
                            readNextLayer(it, curSymbol);
                    }
                }
                else if (curSymbol == "num") {
                    readNextLayer(it, curSymbol);
                    rightHandSide.push_back(curSymbol);
                }
                else if (curSymbol == "id") {
                    readNextLayer(it, curSymbol);
                    if (isRightHandSide) {
                        rightHandSide.push_back(curSymbol);
                    }
                    else
                        leftHandSide = curSymbol;
                }
                readNextLayer(it, curSymbol);
            }
            isExpr = false;
            isRightHandSide = false;

            if (leftHandSide != "")
                checkType(leftHandSide, rightHandSide, scope);

            leftHandSide = "";
            rightHandSide.clear();
        }
    }

    while (!symbolStack.empty()) {
        Symbol s = symbolStack.top();
        symbolStack.pop();
    }
}

void SemanticAnalyzer::readNextLayer(vector<Node>::iterator& it, string& symbol, int readNum)
{
    for (int i = 0; i < readNum; i++)
        ++it;
    symbol = it->symbol;
}

void SemanticAnalyzer::printSymbolTable()
{
    for (auto entry : symbolTable) {
        for (auto s : entry.second)
            cout << s.scope << "\t" << s.symbol << "\t" << s.type << "\t"
                 << s.isArray << "\t" << s.isFunction << endl;
        cout << endl;
    }
}

map<int, vector<Symbol>> SemanticAnalyzer::getSymbolTable()
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
        for (auto entry : symbolTable) {
            for (auto s : entry.second)
                outputFileStream << s.scope << "\t" << s.symbol << "\t"
                                 << s.type << "\t" << s.isArray << "\t"
                                 << s.isFunction << endl;
            outputFileStream << endl;
        }
        outputFileStream.close();
    }
    else {
        cout << "Cannot open file " << fileName << endl;
    }
}

void SemanticAnalyzer::checkType(string left, vector<string> right, int scope)
{
    Symbol firstSymbol, secondSymbol;
    if (right.size() == 0) {
        return;
    }
    else if (right.size() > 1) {
        firstSymbol = Symbol(scope, right[0], getType(scope, right[0]));
        secondSymbol = Symbol(scope, right[1], getType(scope, right[1]));

        if (firstSymbol.type != secondSymbol.type) {
            printTypeWarning(firstSymbol, secondSymbol);
            firstSymbol.type = typeCasting(firstSymbol, secondSymbol);
        }

        firstSymbol.symbol = "temp";
        for (int i = 2; i < right.size(); i++) {
            secondSymbol = Symbol(scope, right[i], getType(scope, right[i]));
            if (firstSymbol.type != secondSymbol.type) {
                printTypeWarning(firstSymbol, secondSymbol);

                firstSymbol.type = typeCasting(firstSymbol, secondSymbol);
            }
        }

        secondSymbol = Symbol(scope, "temp", firstSymbol.type);
    }
    else {
        secondSymbol = Symbol(scope, right[0], getType(scope, right[0]));
    }

    firstSymbol = Symbol(scope, left, getType(scope, left));

    if (firstSymbol.type != secondSymbol.type) {
        printTypeWarning(firstSymbol, secondSymbol);

        firstSymbol.type = typeCasting(firstSymbol, secondSymbol);
    }

    cout << left << " =  ";
    for (auto r : right)
        cout << r << "  ";
    cout << endl;
}

void SemanticAnalyzer::printTypeWarning(Symbol s1, Symbol s2)
{
    cout << "warning (scope " << s1.scope << "): " << s1.symbol << "  "
         << s1.type << ",  " << s2.symbol << "  " << s2.type << endl;
}

string SemanticAnalyzer::getType(int scope, string symbol)
{
    if (isID(symbol))
        return accessSymbolTable(scope, symbol).type;
    else if (isDouble(symbol))
        return "double";
    else
        return "int";
}

string SemanticAnalyzer::typeCasting(Symbol s1, Symbol s2)
{
    if (TYPE_PRIORITY.at(s1.type) > TYPE_PRIORITY.at(s2.type))
        return s1.type;
    else
        return s2.type;
}

Symbol SemanticAnalyzer::accessSymbolTable(int scope, string symbol)
{
    while (scope > -1) {
        vector<Symbol> symbolList = symbolTable[scope];
        for (auto s : symbolList)
            if (s.symbol == symbol)
                return s;
        scope--;
    }
    return Symbol();
}

bool SemanticAnalyzer::isID(string symbol) { return !isdigit(symbol[0]); }

bool SemanticAnalyzer::isDouble(string symbol)
{
    return symbol.find(".") != string::npos;
}

const map<string, int> SemanticAnalyzer::TYPE_PRIORITY{
    {"int", 1}, {"float", 2}, {"double", 3}};
