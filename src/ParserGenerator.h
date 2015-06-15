#ifndef PARSER_GERNERATOR_H
#define PARSER_GERNERATOR_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <map>
#include <set>
#include <vector>
using namespace std;

class ParserGenerator {
public:
    static const string EPSILON;
    static const string START_SYMBOL;
    static const string END_SYMBOL;

    ParserGenerator(string grammarFile = "input/grammar.txt");
    void readGrammar(string fileName);
    void findFirstSet();
    void findFollowSet();
    void generateParseTable();
    void exportSet(string fileName = "set.txt");
    void exportParseTable(string fileName = "LLtable.txt");

    map<string, map<string, vector<string> > > getParseTable();
    set<string> getTerminals();
    set<string> getNonTerminals();

    static bool contain(set<string> s, string st);
    static bool containEpsilon(set<string> s);
    static bool isEpsilon(string symbol);
    static bool isEpsilonProduction(vector<string> production);
    static set<string> excludeEpsilon(set<string> s);

    void printAll();
private:
    vector<string> inputSequence;
    map<string, vector<vector<string> > > grammar;
    set<string> nonTerminals;
    set<string> terminals;
    map<string, set<string> > firstSet;
    map<string, set<string> > followSet;
    map<string, map<string, vector<string> > > parseTable;

    vector<string> split(string str);
    set<string> setDifference(set<string> s1, set<string> s2);
    set<string> setUnion(set<string> s1, set<string> s2);

    bool isTerminal(string symbol);
    bool isNonTerminal(string symbol);
};

#endif
