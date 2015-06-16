#include "Parser.h"
#include "ParserGenerator.h"

Parser::Parser()
{
}

void Parser::setParseTable(map<string, map<string, vector<string> > > table)
{
    parseTable = table;
}

void Parser::generateParseTree(vector<Token> allTokens)
{
    // initialize stack = S instead of S $
    // due to the grammar S->program$
    stack<Node> parseStack;
    parseStack.push(Node(1, ParserGenerator::START_SYMBOL));

    int tokenCounter = 0;
    bool errorOccur = false;
    Node stackTop;
    int layer;
    string symbol;
    string input;
    while (!parseStack.empty()) {
        stackTop = parseStack.top();
        parseStack.pop();

        layer = stackTop.layer;
        symbol = stackTop.symbol;

        if (tokenCounter < allTokens.size())
            if (allTokens[tokenCounter].category == "Identifier")
                input = "id";
            else if (allTokens[tokenCounter].category == "Number")
                input = "num";
            else
                input = allTokens[tokenCounter].content;
        else
            input = ParserGenerator::END_SYMBOL;

        // Add node current parsing to tree
        tree.push_back(stackTop);
        if (isNonTerminal(symbol)) {
            if (!entryExistInParseTable(symbol, input)) {
                errorOccur = true;
                break;
            }
            else {
                if (ParserGenerator::isEpsilonProduction(parseTable[stackTop.symbol][input]))
                    tree.push_back(Node(layer + 1, ParserGenerator::EPSILON));
                else
                    for (vector<string>::reverse_iterator it =
                             parseTable[symbol][input].rbegin();
                         it != parseTable[symbol][input].rend(); ++it)
                        parseStack.push(Node(layer + 1, *it));

            }
        }
        else if (isTerminal(symbol)) {
            if (symbol == input) {
                // Add id's and num's content into parse tree in lower layer
                if (symbol == "id")
                    tree.push_back(
                        Node(layer + 1, allTokens[tokenCounter].content));
                else if (symbol == "num")
                    tree.push_back(
                        Node(layer + 1, allTokens[tokenCounter].content));

                tokenCounter++;
            }
            else {
                errorOccur = true;
                break;
            }
        }
        else {
            errorOccur = true;
            break;
        }
    }
}

void Parser::setTerminals(set<string> t)
{
    terminals = t;
}

void Parser::setNonTerminals(set<string> nt)
{
    nonTerminals = nt;
}

void Parser::exportTree(string fileName)
{
    ofstream outputFilestream(fileName);
    if (outputFilestream.is_open()) {
        for (auto i : tree) {
            for (int j = 0; j < i.layer; j++)
                outputFilestream << " ";
            outputFilestream << left << setw(15) << i.layer
                             << left << setw(15) << i.symbol << endl;
        }
        outputFilestream.close();
    }
    else {
        cout << "Cannot open file" << fileName << endl;
    }
}

bool Parser::isTerminal(string symbol)
{
    return terminals.find(symbol) != terminals.end();
}

bool Parser::isNonTerminal(string symbol)
{
    return nonTerminals.find(symbol) != nonTerminals.end();
}

bool Parser::entryExistInParseTable(string key1, string key2)
{
    bool firstKeyExist = (parseTable.find(key1) != parseTable.end());
    if (firstKeyExist) {
        bool secondKeyExist =
            (parseTable.at(key1).find(key2) != parseTable.at(key1).end());
        if (secondKeyExist)
            return true;
    }
    return false;
}

vector<Node> Parser::getParseTree()
{
    return tree;
}

void Parser::printTree()
{
    for (auto i : tree) {
        for (int j = 0; j < i.layer; j++)
            cout << " ";
        cout << left << setw(15) << i.layer
             << left << setw(15) << i.symbol
             << endl;
    }
}
