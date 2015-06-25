#include "SemanticAnalyzer.h"

void SemanticAnalyzer::analysis(vector<Node> parseTree)
{
    int scope = 0;
    int scopeCounter = 0;

    bool isDecl = false;
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
            bool isAssignment = false;

            // bool isRightHandSide = false;
            // string leftHandSide;
            // vector<string> rightHandSide;
            vector<string> expr;

            readNextLayer(it, curSymbol);
            while (it->layer != treeLayer) {
                if (curSymbol == "ExprIdTail" || curSymbol == "ExprArrayTail") {
                    readNextLayer(it, curSymbol);

                    if (curSymbol == "=") {
                        isAssignment = true;
                        expr.push_back(curSymbol);
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
                else if (curSymbol == "num" || curSymbol == "id" || curSymbol == "BinOp"||
                         curSymbol == "{" || curSymbol == "}") {
                    readNextLayer(it, curSymbol);
                    expr.push_back(curSymbol);
                }
                readNextLayer(it, curSymbol);
            }

            if (expr.size() > 1)
                checkType(expr, scope);

            expr.clear();
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

void SemanticAnalyzer::checkType(vector<string> expr, int scope)
{
    stack<Symbol> s;
    Symbol operand1, operand2, result;
    vector<Symbol> postfixExpr = infixExprToPostfix(expr, scope);
    for (auto sym : postfixExpr) {
        if (sym.type != "op") {
            s.push(sym);
        }
        else {
            operand2 = s.top();
            s.pop();
            operand1 = s.top();
            s.pop();

            if (operand1.type != operand2.type)
                printTypeWarning(scope, operand1, operand2);

            result = Symbol("temp", typeCasting(operand1, operand2));
            s.push(result);
        }
    }

}

void SemanticAnalyzer::printTypeWarning(int scope, Symbol s1, Symbol s2)
{
    cout << "warning (scope " << scope << "): " << s1.symbol << "  "
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

vector<Symbol> SemanticAnalyzer::infixExprToPostfix(vector<string> expr, int scope)
{
    vector<Symbol> postfixExpr;
    stack<string> s;
    Symbol exprSymbol;
    for (auto sym : expr) {
        if (isOperator(sym)) {
            // the lower the prioity value, the higher the prioity
            if (!s.empty() && s.top() != "(") {
                while (OP_PRIORITY.at(s.top()) <= OP_PRIORITY.at(sym)) {
                    exprSymbol.symbol = s.top();
                    exprSymbol.type = "op";
                    postfixExpr.push_back(exprSymbol);
                    s.pop();
                    if (!s.empty() && s.top() == "(")
                        break;
                    else if (s.empty())
                        break;
                }
            }
            s.push(sym);
        }
        else if (sym == "(") {
            s.push(sym);
        }
        else if (sym == ")") {
            while (s.top() != "(") {
                if (s.top() != "(") {
                    exprSymbol.symbol = s.top();
                    exprSymbol.type = "op";
                    postfixExpr.push_back(exprSymbol);
                }
                s.pop();
            }
        }
        else {
            if (isID(sym)) {
                exprSymbol.symbol = sym;
                exprSymbol.type = getType(scope, sym);
            } else {
                exprSymbol.symbol = sym;
                if (isDouble(sym))
                    exprSymbol.type = "double";
                else
                    exprSymbol.type = "int";
            }
            exprSymbol.scope = scope;
            postfixExpr.push_back(exprSymbol);
        }
    }

    while (!s.empty()) {
        if (s.top() != "(") {
            exprSymbol.symbol = s.top();
            exprSymbol.type = "op";
            postfixExpr.push_back(exprSymbol);
        }
        s.pop();
    }

    return postfixExpr;
}

bool SemanticAnalyzer::isOperator(string symbol)
{
    return OP_PRIORITY.find(symbol) != OP_PRIORITY.end();
}

const map<string, int> SemanticAnalyzer::OP_PRIORITY{{"-", 2},
                                                  {"!", 2},
                                                  {"+", 2},
                                                  {"-", 2},
                                                  {"*", 3},
                                                  {"/", 3},
                                                  {"==", 7},
                                                  {"!=", 7},
                                                  {"<", 6},
                                                  {"<=", 6},
                                                  {">", 6},
                                                  {">=", 6},
                                                  {"&&", 11},
                                                  {"||", 12},
                                                  {"=", 15}};
