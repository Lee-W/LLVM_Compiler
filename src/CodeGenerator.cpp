#include "CodeGenerator.h"

const char* CodeGenerator::typeCast(string type)
{
    string ret;
    if (type == "int")
        ret = "i32";
    else if (type == "double")
        ret = "double";
    else if (type == "char")
        ret = "i8";
    else
        printf("Unknown Type found!\n");
    return ret.c_str();
}

Symbol CodeGenerator::findType(vector<Node>::iterator it)
{
    string id = it->symbol;
    for (map<int, vector<Symbol>>::iterator symbols = symbolTable.begin();
         symbols != symbolTable.end(); symbols++) {
        for (vector<Symbol>::iterator go = symbols->second.begin();
             go != symbols->second.end(); go++) {
            if (go->symbol == id)
                return *go;
        }
    }
    return Symbol();
}

// Assuming no variables have the same name
Symbol CodeGenerator::findSymbol(string symbol)
{
    for (auto entry : symbolTable)
        for (auto sym : entry.second)
            if (sym.symbol == symbol)
                return sym;
    return Symbol();
}

void CodeGenerator::printID(vector<Node>::iterator it)
{
    Symbol target = findType(it);
    string id;
    if (target.scope == 0)  // global variable
        id = "@" + it->symbol;
    else
        id = "%" + it->symbol;
    string type = target.type;
    if (type == "int") {
        fprintf(llFile, "%%%d = load i32* %s\n", instruction, id.c_str());
        fprintf(llFile,
                "call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([4 x "
                "i8]* @istr, i32 0, i32 0), i32 %%%d)\n",
                instruction);
    }
    else if (type == "double") {
        fprintf(llFile, "%%%d = load double* %s\n", instruction, id.c_str());
        fprintf(llFile,
                "call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([4 x "
                "i8]* @fstr, i32 0, i32 0), double %%%d)\n",
                instruction);
    }
    instruction += 2;
}

// global variable or function define
void CodeGenerator::declaration(vector<Node>::iterator it)
{
    it = it + 2;
    string type = it->symbol;  // save the Type
    it = it + 2;
    string id = it->symbol;
    it = it + 2;

    if (it->symbol == "VarDecl'") {  // global variable/array declaration
        it++;
        if (it->symbol == ";") {  // global variable
            if (type == "int")
                fprintf(llFile, "@%s = global %s 0\n", id.c_str(),
                        typeCast(type));
            else if (type == "double")
                fprintf(llFile, "@%s = global %s 0.000000e+00\n", id.c_str(),
                        typeCast(type));
            else if (type == "char")
                fprintf(llFile, "@%s = global %s 0\n", id.c_str(),
                        typeCast(type));
        }
        else if (it->symbol == "[") {  // global array
            it = it + 2;
            fprintf(llFile, "@%s = global [%s x %s] zeroinitializer\n",
                    id.c_str(), it->symbol.c_str(), typeCast(type));
        }
    }
    else if (it->symbol == "FunDecl") {  // function declaration
        it = it + 3;
        if (it->symbol == "epsilon")  // No parameters
            fprintf(llFile, "define %s @%s() ", typeCast(type), id.c_str());
        else {  // with parameters
            fprintf(llFile, "define %s @%s(", typeCast(type), id.c_str());
            for (; it->symbol != ")"; it++) {
                if (it->symbol == "ParamDecl") {
                    it = it + 2;
                    fprintf(llFile, "%s ", typeCast(it->symbol));
                    it = it + 2;
                    fprintf(llFile, "%%%s", it->symbol.c_str());
                }
                else if (it->symbol == ",")
                    fprintf(llFile, ",");
            }
            fprintf(llFile, ")");
        }
    }
}

void CodeGenerator::varDecl(vector<Node>::iterator it)
{  // local variable declartion
    it = it + 2;
    string type = it->symbol;  // save the Type
    it = it + 2;
    string id = it->symbol;  // save the id
    it = it + 2;
    if (it->symbol == ";") {  // variable
        fprintf(llFile, "%%%s = alloca %s\n", id.c_str(), typeCast(type));
    }
    else if (it->symbol == "[") {  // array
        it = it + 2;
        fprintf(llFile, "%%%s = alloca [%s x %s]\n", id.c_str(),
                it->symbol.c_str(), typeCast(type));
    }
}

void CodeGenerator::expr(vector<Node>::iterator it)  // start calculation
{
    int exprLayer = it->layer;  // save this expr's level
    it++;
    vector<string> expression;
    for (; it->layer > exprLayer; it++) {
        string thisSymbol = it->symbol;
        if (thisSymbol == "+" || thisSymbol == "-" || thisSymbol == "*" ||
            thisSymbol == "/" || thisSymbol == "==" || thisSymbol == "!=" ||
            thisSymbol == "<" || thisSymbol == "<=" || thisSymbol == ">" ||
            thisSymbol == ">=" || thisSymbol == "&&" || thisSymbol == "||" ||
            thisSymbol == "=")  // BinOp
            expression.push_back(thisSymbol);
        else if (thisSymbol == "id") {
            it++;
            string var = it->symbol;
            // it += 2;
            expression.push_back(it->symbol);
            /*
            if (it->symbol != "(" && it->symbol != "[")	//function
                    expression.push_back(var);
            */
        }
        else if (thisSymbol == "num") {
            it++;
            expression.push_back(it->symbol);
        }
    }
    for (vector<string>::iterator x = expression.begin(); x != expression.end();
         x++)
        cout << *x << " ";
    cout << endl;
}

void CodeGenerator::statement(vector<Node>::iterator it)
{
    vector<Node>::iterator temp = ++it;
    if (temp->symbol == "print") {  // print id;
        temp = temp + 2;
        printID(temp);
    }
    else if (temp->symbol == "Expr") {  // Expr;
        expr(it);
    }
    else if (temp->symbol == "if") {  // if ( Expr ) Stmt else Stmt
        ifElse(it);
    }
    else if (temp->symbol == "while") {  // while ( Expr ) Stmt
        whileStatement(it);
    }
}

void CodeGenerator::whileStatement(vector<Node>::iterator it)
{
    string cmp;
    int exprLabel, stmtLabel, originLabel;
    exprLabel = ++instruction;
    stmtLabel = ++instruction;
    originLabel = ++instruction;

    fprintf(llFile, "br label %%%d\n", exprLabel);

    fprintf(
        llFile,
        "\n; <label>:%%%d                                       ; preds = %%0\n",
        exprLabel);
    it += 2;
    expr(it);
    // TODO: assign cmp
    fprintf(llFile, "br i1 %%%s, label %%%d, label %%%d\n", cmp.c_str(), stmtLabel,
            originLabel);

    it++;
    fprintf(
        llFile,
        "\n; <label>:%%%d                                       ; preds = %%0\n",
        stmtLabel);
    statement(it);
    fprintf(llFile, "br label %%%d\n", exprLabel);

    fprintf(
        llFile,
        "\n; <label>:%%%d                                       ; preds = %%0\n",
        originLabel);
}

void CodeGenerator::ifElse(vector<Node>::iterator it)
{
    string cmp;
    int ifLabel, elseLabel, originLabel;
    ifLabel = ++instruction;
    elseLabel = ++instruction;
    originLabel = ++instruction;

    it += 2;
    expr(it);
    // TODO : assign cmp

    fprintf(llFile, "br i1 %%%s, label %%%d, label %%%d\n", cmp.c_str(),
            ifLabel, elseLabel);

    it++;
    fprintf(
        llFile,
        "\n; <label>:%%%d                                       ; preds = %%0\n",
        ifLabel);
    statement(it);
    fprintf(llFile, "br label %%%d\n", originLabel);

    it++;
    fprintf(
        llFile,
        "\n; <label>:%%%d                                       ; preds = %%0\n",
        elseLabel);
    statement(it);
    fprintf(llFile, "br label %%%d\n", originLabel);

    fprintf(
        llFile,
        "\n; <label>:%%%d                                       ; preds = %%0\n",
        originLabel);
}

void CodeGenerator::codeGeneration(vector<Node> parseTree)
{
    llFile = fopen("output.ll", "w");

    // set up print function
    fprintf(llFile, "@istr = private constant[4 x i8] c\"%%d\\0A\\00\"\n");
    fprintf(llFile, "@fstr = private constant[4 x i8] c\"%%f\\0A\\00\"\n");
    fprintf(llFile, "declare i32 @printf(i8*, ...)\n\n");

    // Some flag

    // scan the tree to generate the code
    for (vector<Node>::iterator it = parseTree.begin(); it != parseTree.end();
         it++) {
        // Start analyzing the tree
        if (it->symbol == "DeclList'")  // declaration of variable or function
            declaration(it);
        else if (it->symbol ==
                 "VarDecl")  // declaration of variable (could be array)
            varDecl(it);
        else if (it->symbol == "Stmt")  // statement is appeared
            statement(it);
        else if (it->symbol == "{")
            fprintf(llFile, "{\n");
        else if (it->symbol == "}")
            fprintf(llFile, "}\n");
    }
    fclose(llFile);
}

void CodeGenerator::testFunctions()
{
    vector<string> tt;
    tt.push_back("a");
    tt.push_back("=");
    tt.push_back("(");
    tt.push_back("b");
    tt.push_back("+");
    tt.push_back("c");
    tt.push_back(")");
    tt.push_back("+");
    tt.push_back("fun(a, b)");
    tt.push_back("!a");
    for (auto sym : infixExprToPostfix(tt)) {
        cout << sym.symbol << "\t" << sym.type << endl;
    }
    handleExpr(infixExprToPostfix(tt));
}

void CodeGenerator::handleExpr(vector<Symbol> expr)
{
    stack<Symbol> s;
    Symbol operand1, operand2, result;
    for (auto sym : expr) {
        if (sym.type != "op") {
            s.push(sym);
        }
        else {
            operand2 = s.top();
            s.pop();
            if (operand2.isFunction) {
                // TODO: handle expr in function
            }
            else if (operand2.isArray) {
                // TODO: handle expr in array
            }

            operand1 = s.top();
            s.pop();
            if (operand1.isFunction) {
                // TODO: handle expr in function
            }
            else if (operand1.isArray) {
                // TODO: handle expr in array
            }

            cout << "Expr:   " << operand1.symbol << "\t" << sym.symbol << "\t"
                 << operand2.symbol << endl;
            if (sym.symbol == "+") {
                // TODO: generate llvm code
            }
            else if (sym.symbol == "-") {
                // TODO: generate llvm code
            }
            else if (sym.symbol == "*") {
                // TODO: generate llvm code
            }
            else if (sym.symbol == "/") {
                // TODO: generate llvm code
            }
            else if (sym.symbol == "==") {
                // TODO: generate llvm code
            }
            else if (sym.symbol == "!=") {
                // TODO: generate llvm code
            }
            else if (sym.symbol == "<") {
                // TODO: generate llvm code
            }
            else if (sym.symbol == "<=") {
                // TODO: generate llvm code
            }
            else if (sym.symbol == ">") {
                // TODO: generate llvm code
            }
            else if (sym.symbol == ">") {
                // TODO: generate llvm code
            }
            else if (sym.symbol == "&&") {
                // TODO: generate llvm code
            }
            else if (sym.symbol == "||") {
                // TODO: generate llvm code
            }
            else if (sym.symbol == "=") {
                // TODO: generate llvm code
            }
            // TODO: assign meaningful symbol instead of temp
            result = Symbol("temp", "Test");
            s.push(result);
        }
    }
}

vector<Symbol> CodeGenerator::infixExprToPostfix(vector<string> expr)
{
    vector<Symbol> prefixExpr;
    stack<string> s;
    Symbol exprSymbol;

    for (auto sym : expr) {
        if (isOperator(sym)) {
            // the lower the prioity value, the higher the prioity
            if (!s.empty() && s.top() != "(") {
                while (OP_PRIORITY.at(s.top()) <= OP_PRIORITY.at(sym)) {
                    exprSymbol = findSymbol(s.top());
                    exprSymbol.symbol = s.top();
                    exprSymbol.type = "op";
                    prefixExpr.push_back(exprSymbol);
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
                    exprSymbol = findSymbol(s.top());
                    exprSymbol.symbol = s.top();
                    exprSymbol.type = "op";
                    prefixExpr.push_back(exprSymbol);
                }
                s.pop();
            }
        }
        else {
            string symbolID;
            if (sym.find("[") != string::npos)
                symbolID = sym.substr(0, sym.find("["));
            else if (sym.find("(") != string::npos)
                symbolID = sym.substr(0, sym.find("("));
            else if (sym.find("!") != string::npos)
                symbolID = sym.substr(sym.find("!") + 1);
            else if (sym.find("-") != string::npos)
                symbolID = sym.substr(sym.find("-") + 1);
            else
                symbolID = sym;
            exprSymbol = findSymbol(symbolID);
            exprSymbol.symbol = sym;
            prefixExpr.push_back(exprSymbol);
        }
    }

    while (!s.empty()) {
        if (s.top() != "(") {
            exprSymbol = findSymbol(s.top());
            exprSymbol.symbol = s.top();
            exprSymbol.type = "op";
            prefixExpr.push_back(exprSymbol);
        }
        s.pop();
    }

    return prefixExpr;
}

bool CodeGenerator::isOperator(string symbol)
{
    return OP_PRIORITY.find(symbol) != OP_PRIORITY.end();
}

void CodeGenerator::setSymbolTable(map<int, vector<Symbol>> st)
{
    symbolTable = st;
}

const map<string, int> CodeGenerator::OP_PRIORITY{{"-", 2},
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
