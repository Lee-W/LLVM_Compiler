#include "CodeGenerator.h"

void CodeGenerator::codeGeneration(vector<Node> parseTree)
{
    // set up print function
    llvmCode.push_back("@istr = private constant[4 x i8] c\"%d\\0A\\00\"\n");
    llvmCode.push_back("@fstr = private constant[4 x i8] c\"%f\\0A\\00\"\n");
    llvmCode.push_back("declare i32 @printf(i8*, ...)\n\n");

    // scan the tree to generate the code
    for (vector<Node>::iterator it = parseTree.begin(); it != parseTree.end();
         it++) {
        // Start analyzing the tree
        if (it->symbol == "DeclList'")  // declaration of variable or function
            appendVectors(llvmCode, declaration(it));
        else if (it->symbol == "VarDecl")  // declaration of variable (could be array)
            appendVectors(llvmCode,varDecl(it));
        else if (it->symbol == "Stmt")  // statement is appeared
            appendVectors(llvmCode, statement(it));
        else if (it->symbol == "{")
            llvmCode.push_back("{\n");
        else if (it->symbol == "}")
            llvmCode.push_back("}\n");
    }
    exportLlvmCode();
}

// global variable or function define
vector<string> CodeGenerator::declaration(vector<Node>::iterator& it)
{
    vector<string> declCode;

    it = it + 2;
    string type = it->symbol;  // save the Type
    it = it + 2;
    string id = it->symbol;
    it = it + 2;

    stringstream line;
    if (it->symbol == "VarDecl'") {  // global variable/array declaration
        it++;
        if (it->symbol == ";") {  // global variable
            if (type == "int")
                line << "@" << id << " = global "<< typeCast(type)<<" 0\n";
            else if (type == "double")
                line << "@" << id << " = global "<< typeCast(type) << " 0.000000e+00\n";
            else if (type == "char")
                line << "@" << id << " = global "<< typeCast(type)<<" 0\n";
        }
        else if (it->symbol == "[") {  // global array
            it = it + 2;
            line << "@" << id << " = global [" << it->symbol << " x " << typeCast(type) << "] zeroinitializer\n";
        }
    }
    else if (it->symbol == "FunDecl") {  // function declaration
        it = it + 3;
        if (it->symbol == "epsilon")  // No parameters
            line << "define " << typeCast(type) << " @" << id << "() ";
        else {  // with parameters
            line << "define " << typeCast(type) << " @" << id << "(";
            for (; it->symbol != ")"; it++) {
                if (it->symbol == "ParamDecl") {
                    it = it + 2;
                    line << typeCast(it->symbol) << " ";
                    it = it + 2;
                    line << "%" << it->symbol;
                }
                else if (it->symbol == ",")
                    line << ",";
            }
            line << ")";
        }
    }
    declCode.push_back(line.str());
    return declCode;
}

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

// local variable declartion
vector<string> CodeGenerator::varDecl(vector<Node>::iterator& it)
{
    vector<string> varDeclCode;
    stringstream line;

    it = it + 2;
    string type = it->symbol;  // save the Type
    it = it + 2;
    string id = it->symbol;  // save the id
    it = it + 2;
    if (it->symbol == ";") {  // variable
        line << "%" << id << " = alloca " <<typeCast(type) << "\n";
    }
    else if (it->symbol == "[") {  // array
        it = it + 2;
        line << "%" << id << " = alloca [" << it->symbol << " x " << typeCast(type) << "]\n";
    }
    varDeclCode.push_back(line.str());
    return varDeclCode;
}

vector<string> CodeGenerator::statement(vector<Node>::iterator& it)
{
    vector<string> stmtCode;

    vector<Node>::iterator temp = ++it;
    if (temp->symbol == "print") {  // print id;
        temp = temp + 2;
        appendVectors(stmtCode, printID(temp));
        it += 3;
    }
    else if (temp->symbol == "Expr") {  // Expr;
        appendVectors(stmtCode, expr(it));
        ++it;
    }
    else if (temp->symbol == "if") {  // if ( Expr ) Stmt else Stmt
        appendVectors(stmtCode, ifElse(it));
    }
    else if (temp->symbol == "while") {  // while ( Expr ) Stmt
        appendVectors(stmtCode, whileStatement(it));
    } else if (temp->symbol == "return") {
        // TODO: return 
        string ret = "ret i32 0\n";
        stmtCode.push_back(ret);
    } else if (temp->symbol == "Block") {
        appendVectors(stmtCode, block(it));
    }

    return stmtCode;
}

vector<string> CodeGenerator::printID(vector<Node>::iterator& it)
{
    vector<string> printCode;
    stringstream line;

    Symbol target = findType(it);
    string id;
    if (target.scope == 0)  // global variable
        id = "@" + it->symbol;
    else
        id = "%" + it->symbol;
    string type = target.type;
    instruction++;
    if (type == "int") {
        line << "%" << instruction << " = load i32* " << id << "\n";
        line << "call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([4 x " <<
                "i8]* @istr, i32 0, i32 0), i32 %" << instruction << ")\n" ;
    }
    else if (type == "double") {
        line << "%" << instruction << " = load double* " << id << "\n";
        line << "call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([4 x "
                "i8]* @fstr, i32 0, i32 0), double %"<< instruction <<")\n";
    }
    printCode.push_back(line.str());
    instruction += 2;

    return printCode;
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

vector<string> CodeGenerator::expr(vector<Node>::iterator& it)  // start calculation
{
    // TODO : add value to exprCode
    vector<string> exprCode;

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
        // else if (thisSymbol == "(") {
        //     expression.push_back(it->symbol);
        // }
        // else if (thisSymbol == ")") {
        //     expression.push_back(it->symbol);
        // }
    }
    it--;
//    for (vector<string>::iterator x = expression.begin(); x != expression.end();
//         x++)
//        cout << *x << " ";
//    cout << endl;
	appendVectors(exprCode, handleExpr(infixExprToPostfix(expression)));
    return exprCode;
}

vector<string> CodeGenerator::ifElse(vector<Node>::iterator& it)
{
    vector<string> code;
    stringstream line;

    vector<string> exprCode, stmtCode1, stmtCode2;

    string cmp;
    int trueLabel, falseLabel, originLabel;

    it += 2;
    exprCode = expr(it);

    cmp = exprCode.back().substr(0, exprCode.back().find(" "));
    trueLabel = ++instruction;

    it += 2;
    stmtCode1 = statement(it);
    falseLabel = ++instruction;

    it += 2;
    stmtCode2 = statement(it);
    originLabel = ++instruction;


    appendVectors(code, exprCode);

    line << "br i1 " << cmp << ", label %" << trueLabel << ", label %" << falseLabel <<"\n";
    code.push_back(line.str());
    line.str("");

    line << "\n; <label>:" << trueLabel << "\n";
    code.push_back(line.str());
    line.str("");
    appendVectors(code, stmtCode1);
    line << "br label %" << originLabel << "\n";
    code.push_back(line.str());
    line.str("");

    line << "\n; <label>:" << falseLabel << "\n";
    code.push_back(line.str());
    line.str("");
    appendVectors(code, stmtCode2);
    line << "br label %" << originLabel << "\n";
    code.push_back(line.str());
    line.str("");

    line << "\n; <label>:" << originLabel << "\n";
    code.push_back(line.str());
    line.str("");

    return code;
}

vector<string> CodeGenerator::whileStatement(vector<Node>::iterator& it)
{
    vector<string> code;
    stringstream line;

    vector<string> exprCode, stmtCode;

    string cmp;
    int exprLabel, stmtLabel, originLabel;

    exprLabel = ++instruction;
    it += 2;
    exprCode = expr(it);
    cmp = exprCode.back().substr(0, exprCode.back().find(" "));

    stmtLabel = ++instruction;
    it += 2;
    stmtCode = statement(it);

    originLabel = ++instruction;


    line << "br label %" << exprLabel << "\n";
    code.push_back(line.str());
    line.str("");

    line << "\n; <label>:" << exprLabel << "\n";
    code.push_back(line.str());
    line.str("");
    appendVectors(code, exprCode);
    line << "br i1 " << cmp << ", label %" << stmtLabel << ", label %" << originLabel <<"\n";
    code.push_back(line.str());
    line.str("");

    line << "\n; <label>:" << stmtLabel << "\n";
    code.push_back(line.str());
    line.str("");
    appendVectors(code, stmtCode);
    line << "br label %" << exprLabel << "\n";
    code.push_back(line.str());
    line.str("");

    line << "\n; <label>:" << originLabel << "\n";
    code.push_back(line.str());
    line.str("");

    return code;
}

vector<string> CodeGenerator::handleExpr(vector<Symbol> expr)
{
    vector<string> exprCode;
    stringstream line;

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
            else if (operand2.type == "") {  // it is constant
                operand2.isConstant = true;
                if (operand2.symbol.find(".", 0) == string::npos) {  // it is
                                                                     // int
                    operand2.type = "int";
                    instruction++;

                    line << "%" << instruction << " = add i32 0, " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    operand2.symbol = "%" + to_string(instruction);
                }
                else {
                    // need to cast to LLVM double type
                    operand2.type = "double";
                    instruction++;


                    line << "%" << instruction << " = fpext float " << operand2.symbol << " to double\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    operand2.symbol = "%" + to_string(instruction);
                }
            }
            else {  // variable
                if (operand2.scope == 0) {  // global variable
                    instruction++;

                    line << "%" << instruction << " = load " << typeCast(operand2.type) << "* @"<<operand2.symbol<<"\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    operand2.symbol = "%" + to_string(instruction);
                }
                else {  // local variable
                    instruction++;

                    line << "%" << instruction << " = load " << typeCast(operand2.type) << "* %" << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    operand2.symbol = "%" + to_string(instruction);
                }
            }

            operand1 = s.top();
            s.pop();
            if (operand1.isFunction) {
                // TODO: handle expr in function
            }
            else if (operand1.isArray) {
                // TODO: handle expr in array
            }
            else if (operand1.type == "") {  // it is constant
                operand1.isConstant = true;
                if (operand1.symbol.find(".", 0) == string::npos) {  // it is
                                                                     // int
                    operand1.type = "int";
                    instruction++;

                    line << "%" << instruction << " = add i32 0, " << operand1.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    operand1.symbol = "%" + to_string(instruction);
                }
                else {
                    operand1.type =
                        "double";  // need to cast to LLVM double type
                    instruction++;
                    line << "%" << instruction << " = fpext float " << operand1.symbol << " to double\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    operand1.symbol = "%" + to_string(instruction);
                }
            }
            else if (sym.symbol != "=") {  // variable
                if (operand1.scope == 0) {  // global variable
                    instruction++;

                    line << "%" << instruction << " = load " << typeCast(operand1.type) << "* @" << operand1.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    operand1.symbol = "%" + to_string(instruction);
                }
                else {  // local variable
                    instruction++;

                    line << "%" << instruction << " = load " << typeCast(operand1.type) << "* @" << operand1.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    operand1.symbol = "%" + to_string(instruction);
                }
            }

            // type conversion if needed
            if (operand1.type != operand2.type && sym.symbol != "=") {
                if (operand1.type == "int") {  // operand1 type conversion
                    instruction++;


                    line << "%" << instruction << " = sitofp i32 " << operand1.symbol <<" to double\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    operand1.symbol = "%" + to_string(instruction);
                    operand1.type = "double";
                }
                else if (operand2.type == "int") {  // operand2 type conversion
                    instruction++;

                    line << "%" << instruction << " = sitofp i32 " << operand2.symbol <<" to double\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    operand2.symbol = "%" + to_string(instruction);
                    operand2.type = "double";
                }
            }

            instruction++;
            // cout << "Expr:   " << operand1.symbol << "\t" << sym.symbol << "\t"
                 // << operand2.symbol << endl;
            if (sym.symbol == "+") {
                if (operand1.type == "int") {
                    line << "%" << instruction << " = add nsw i32 " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "int";
                }
                else if (operand1.type == "double") {
                    line << "%" << instruction << " = fadd double " << operand1.symbol <<", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "double";
                }
            }
            else if (sym.symbol == "-") {
                if (operand1.type == "int") {
                    line << "%" << instruction << " = sub nsw i32 " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "int";
                }
                else if (operand1.type == "double") {
                    line << "%" << instruction << " = fsub double " << operand1.symbol << ", " << operand2.symbol <<"\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "double";
                }
            }
            else if (sym.symbol == "*") {
                if (operand1.type == "int") {
                    line << "%" << instruction << " = mul nsw i32 " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "int";
                }
                else if (operand1.type == "double") {
                    line << "%" << instruction << " = fmul double " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "double";
                }
            }
            else if (sym.symbol == "/") {
                if (operand1.type == "int") {
                    line << "%" << instruction << " = fsub double " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "double";
                }
            }
            else if (sym.symbol == "*") {
                if (operand1.type == "int") {
                    line << "%" << instruction << " = mul nsw i32 " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "int";
                }
                else if (operand1.type == "double") {
                    line << "%" << instruction << " = fmul double " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "double";
                }
            }
            else if (sym.symbol == "/") {
                if (operand1.type == "int") {
                    line << "%" << instruction << " = sdiv i32 " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "int";
                }
                else if (operand1.type == "double") {
                    line << "%" << instruction << " = fdiv double " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "double";
                }
            }
            else if (sym.symbol == "==") {
                if (operand1.type == "int") {
                    line << "%" << instruction << " = icmp eq i32 " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "int";
                }
                else if (operand1.type == "double") {
                    line << "%" << instruction << " = fcmp oeq double " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "double";
                }
            }
            else if (sym.symbol == "!=") {
                if (operand1.type == "int") {
                    line << "%" << instruction << " = icmp ne i32 " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "int";
                }
                else if (operand1.type == "double") {
                    line << "%" << instruction << " = fcmp one double " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "double";
                }
            }
            else if (sym.symbol == "<") {
                if (operand1.type == "int") {
                    line << "%" << instruction << " = icmp slt i32 " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "int";
                }
                else if (operand1.type == "double") {
                    line << "%" << instruction << " = fcmp olt double " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "double";
                }
            }
            else if (sym.symbol == "<=") {
                if (operand1.type == "int") {
                    line << "%" << instruction << " = icmp sle i32 " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "int";
                }
                else if (operand1.type == "double") {
                    line << "%" << instruction << " = fcmp ole double " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "double";
                }
            }
            else if (sym.symbol == ">") {
                if (operand1.type == "int") {
                    line << "%" << instruction << " = icmp sgt i32 " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "int";
                }
                else if (operand1.type == "double") {
                    line << "%" << instruction << " = fcmp ogt double " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "double";
                }
            }
            else if (sym.symbol == ">=") {
                if (operand1.type == "int") {
                    line << "%" << instruction << " = icmp sge i32 " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "int";
                }
                else if (operand1.type == "double") {
                    line << "%" << instruction << " = fcmp oge double " << operand1.symbol << ", " << operand2.symbol << "\n";
                    exprCode.push_back(line.str());
                    line.str("");

                    result.symbol = "%" + to_string(instruction);
                    result.type = "double";
                }
            }
            else if (sym.symbol == "&&") {
                line << "%" << instruction << " = and i32 " << operand1.symbol << ", " << operand2.symbol << "\n";
                exprCode.push_back(line.str());
                line.str("");

                result.symbol = "%" + to_string(instruction);
                result.type = "int";
            }
            else if (sym.symbol == "||") {
                line << "%" << instruction << " = or i32 " << operand1.symbol << ", " << operand2.symbol << "\n";
                exprCode.push_back(line.str());
                line.str("");

                result.symbol = "%" + to_string(instruction);
                result.type = "int";
            }
            else if (sym.symbol == "=") {
                if (operand1.type != operand2.type) {		//type conversion
                	if (operand1.type == "int") {
                		line << "%" << instruction << " = fptosi double " << operand2.symbol << " to i32" << "\n";
                		exprCode.push_back(line.str());
                		line.str("");
                		operand2.symbol = "%" + to_string(instruction);
                	} else if (operand1.type == "double") {
                		line << "%" << instruction << " = sitofp i32 " << operand2.symbol << " to double" << "\n";
                		exprCode.push_back(line.str());
                		line.str("");
                		operand2.symbol = "%" + to_string(instruction);
                	}
                }
                else 
                	instruction--;
                if (operand1.scope == 0)
                	line << "store " << typeCast(operand1.type) << " " << operand2.symbol << ", " << typeCast(operand1.type) << "* @" << operand1.symbol << "\n";
                else
                	line << "store " << typeCast(operand1.type) << " " << operand2.symbol << ", " << typeCast(operand1.type) << "* %" << operand1.symbol << "\n";
                exprCode.push_back(line.str());
                line.str("");
            }
            
            s.push(result);
        }
    }
    return exprCode;
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

vector<string> CodeGenerator::block(vector<Node>::iterator& it)
{
    vector<string> blockCode;

    ++it;

    ++it;
    while (it->symbol == "VarDeclList") {
        ++it;
        if (it->symbol == "VarDecl") {
            appendVectors(blockCode, varDecl(it));
            ++it;
        }
    }

    ++it;
    while (it->symbol == "StmtList") {
        ++it;
        appendVectors(blockCode, statement(it));
        it += 2;
    }
    ++it;
    // cout << "block " << it->symbol << endl;

    return blockCode;
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

void CodeGenerator::appendVectors(vector<string>& v1, vector<string> v2)
{
    v1.reserve(v1.size() + v2.size());
    v1.insert(v1.end(), v2.begin(), v2.end());
}

bool CodeGenerator::isOperator(string symbol)
{
    return OP_PRIORITY.find(symbol) != OP_PRIORITY.end();
}

void CodeGenerator::setSymbolTable(map<int, vector<Symbol>> st)
{
    symbolTable = st;
}

void CodeGenerator::exportLlvmCode(string fileName)
{
    ofstream outputFilestream(fileName);
    if (outputFilestream.is_open()) {
        for (auto line : llvmCode) {
            outputFilestream << line;
        }
        outputFilestream.close();
    }
    else {
        cout << "Cannot open file " << fileName << endl;
    }
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
