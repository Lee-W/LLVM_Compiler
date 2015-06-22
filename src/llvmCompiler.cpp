#include "ParserGenerator.h"
#include "Parser.h"
#include "Lexer.h"
#include "SemanticAnalyzer.h"
using namespace std;

vector<Node> parseTree;
vector<Token> allTokens;

void readProgram(string fileName);
void setupParseTree(string grammarFile, string programFile);
void codeGeneration(vector<Node>, map<int, vector<Symbol>>); 

int main(int argc, const char *argv[])
{
    if (argc != 3) {
        cout << "Argument error" << endl;
    } else {
        setupParseTree(argv[1], argv[2]);

        SemanticAnalyzer sa;
        sa.analysis(parseTree);
        // sa.printSymbolTable();
        sa.exportSymbolTable();
        codeGeneration(parseTree, sa.getSymbolTable());
    }

    return 0;
}

void readProgram(string fileName)
{
    Lexer le;
    vector<vector<Token> > tokenList;

    ifstream inputFileStream(fileName);
    string allInput, line;
    while(getline(inputFileStream, line))
        allInput += line + "\n";
    le.feed(allInput);
    tokenList = le.getResult();
    for (auto line : tokenList)
        for (auto t : line)
            allTokens.push_back(t);
}

void setupParseTree(string grammarFile, string programFile)
{
    ParserGenerator pg(grammarFile);
    Parser p;

    pg.findFirstSet();
    pg.findFollowSet();
    pg.generateParseTable();

    readProgram(programFile);

    p.setParseTable(pg.getParseTable());
    p.setTerminals(pg.getTerminals());
    p.setNonTerminals(pg.getNonTerminals());
    p.generateParseTree(allTokens);
    parseTree = p.getParseTree();
    p.exportTree();
}

