#include "SemanticAnalyzer.h"

void SemanticAnalyzer::analysis(vector<Node> parseTree)
{
    int scope = 0;
    int level = 0;

    bool isDecl = false;
    bool isParDecl = false;
    bool isFunDecl = false;

    string symbol;
    string type;
    bool isArray = false;

    string curSymbol;
    for (auto it = parseTree.begin(); it != parseTree.end(); ++it) {
        curSymbol = it->symbol;

        if (curSymbol == "id") {
            ++it;
            curSymbol = it->symbol;

            symbol = curSymbol;
        } else if (curSymbol == "Type") {
            ++it;
            curSymbol = it->symbol;

            type = curSymbol;
            isDecl = true;
        } else if (curSymbol == "FunDecl") {
            isFunDecl = true;
            cout << scope << "\t" << symbol << "\t" << type << "\t" << isArray << "\t" << isFunDecl << endl;

            isFunDecl = false;
        } else if (curSymbol == "VarDecl'") {
            ++it;
            curSymbol = it->symbol;

            if (curSymbol == "[")
                isArray = true;
            else if (curSymbol == ";" && isDecl) {
                cout << scope << "\t" << symbol << "\t" << type << "\t" << isArray << "\t" << isFunDecl << endl;
                isDecl = false;
                isFunDecl = false;
                isArray = false;
            }
        } else if (curSymbol == ";" && isDecl) {
            cout << scope << "\t" << symbol << "\t" << type << "\t" << isArray << "\t" << isFunDecl << endl;

            isDecl = false;
            isArray = false;
            isFunDecl = false;
        } else if (curSymbol == "ParamDecl") {
            isParDecl = true;

        } else if (curSymbol == "ParamDecl'") {
            ++it;
            curSymbol = it->symbol;

            if (curSymbol == "[")
                isArray = true;
            else if (curSymbol == "epsilon") {
                cout << scope << "\t" << symbol << "\t" << type << "\t" << isArray << "\t" << isFunDecl << endl;

                isParDecl = false;
                isArray = false;
            }
        } else if (curSymbol == "{") {
            level++;
            cout << level << endl;
        } else if (curSymbol == "}") {
            level--;
            cout << level << endl;
        }
    }
}

