#include "ParserGenerator.h"
#include "Parser.h"
#include "Lexer.h"
using namespace std;


map<string, map<string,  vector<string> > > parseTable;
vector<Token> allTokens;

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

int main(int argc, const char *argv[])
{
    if (argc != 3) {
        cout << "Usage: ./Syntax.out [grammar file] [program]" << endl;
    } else {
        ParserGenerator pg(argv[1]);
        Parser p;

        pg.findFirstSet();
        pg.findFollowSet();
        pg.generateParseTable();

        readProgram(argv[2]);

        p.setParseTable(pg.getParseTable());
        p.setTerminals(pg.getTerminals());
        p.setNonTerminals(pg.getNonTerminals());
        p.generateParseTree(allTokens);
        p.printTree();
    }

    return 0;
}

