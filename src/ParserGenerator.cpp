#include "ParserGenerator.h"

ParserGenerator::ParserGenerator(string grammarFile)
{
    readGrammar(grammarFile);
}

void ParserGenerator::readGrammar(string fileName)
{
    string curNonTerminal;

    string line;
    ifstream inputFileStream;
    inputFileStream.open(fileName);
    if (inputFileStream.is_open()) {
        while (getline(inputFileStream, line)) {
            if (!isspace(line[0])) {
                // non terminal
                curNonTerminal = line;
                grammar[line] = vector<vector<string> >();

                nonTerminals.insert(line);
                inputSequence.push_back(line);
            }
            else {
                // productions
                vector<string> production = split(line);
                grammar[curNonTerminal].push_back(production);
                for (auto symbol : production)
                    terminals.insert(symbol);
            }
        }
        // remove non terminals that are previously added to terminals
        terminals = setDifference(terminals, nonTerminals);
        terminals.erase(EPSILON);
    } else {
        cout << "Cannot open file " << fileName << endl;
    }
}

vector<string> ParserGenerator::split(string str)
{
    vector<string> splitedStr;
    stringstream ss(str);
    string buf;
    while (ss >> buf)
        splitedStr.push_back(buf);
    return splitedStr;
}

set<string> ParserGenerator::setDifference(set<string> s1, set<string> s2)
{
    for (auto i : s2) {
        if (s1.find(i) != s1.end())
            s1.erase(i);
    }
    return s1;
}

set<string> ParserGenerator::setUnion(set<string> s1, set<string> s2)
{
    s1.insert(s2.begin(), s2.end());
    return s1;
}

void ParserGenerator::generateParseTable()
{
    bool allContainEplision;
    string deriver;
    vector<vector<string> > productions;
    set<string> tmpFirstSet;
    for (auto entry : grammar) {
        deriver = entry.first;
        productions = entry.second;
        // A -> a
        // t is terminal
        for (auto rule : productions) {
            // for each t in first(a), T[A, t] = a
            tmpFirstSet.clear();
            allContainEplision = true;
            for (auto symbol : rule) {
                tmpFirstSet = setUnion(tmpFirstSet, firstSet[symbol]);
                if (!containEpsilon(firstSet[symbol])) {
                    allContainEplision = false;
                    break;
                }
            }
            // add terminals in tmpFirstSet to parseTable
            for (auto symbol : tmpFirstSet)
                if (!isEpsilon(symbol))
                    parseTable[deriver][symbol] = rule;

            // if eplison belongs to first(a)
            // for each t in follow(A), T[A, t] = a
            if (allContainEplision || isEpsilonProduction(rule)) {
                for (auto symbol : followSet[deriver])
                    parseTable[deriver][symbol] = rule;

                // if $ belongs to follow(A)
                // T[A, $] = a
                if (contain(followSet[deriver], END_SYMBOL))
                    parseTable[deriver][END_SYMBOL] = rule;
            }
        }
    }
}

bool ParserGenerator::isEpsilonProduction(vector<string> production)
{
    return (production.size() == 1 && production.front() == EPSILON);
}

bool ParserGenerator::contain(set<string> s, string st)
{
    return s.find(st) != s.end();
}

void ParserGenerator::exportParseTable(string fileName)
{
    ofstream outputFileStream(fileName);
    if (outputFileStream.is_open()) {
        outputFileStream << START_SYMBOL << endl;
        for (auto i : inputSequence)
            for (auto j : parseTable[i]) {
                outputFileStream << left << setw(20) << i
                                 << left << setw(10) << j.first;
                for (auto k : j.second)
                    outputFileStream << k << " ";
                outputFileStream << endl;
            }
        outputFileStream.close();
    }
    else {
        cout << "Cannot open file " << fileName << endl;
    }
}

void ParserGenerator::findFirstSet()
{
    // set terminals' first set to itself
    for (auto terminal : terminals)
        firstSet[terminal].insert(terminal);

    // Set eplison to deriver's first set if any of its productions
    // produce only EPSILON
    // Set terminal to deriver first set if it's the first symbol in production
    string deriver;
    vector<vector<string> > productions;
    string firstSymbol;
    for (auto entry : grammar) {
        deriver = entry.first;
        productions = entry.second;
        for (auto rule : productions) {
            firstSymbol = rule.front();
            if (isTerminal(firstSymbol))
                firstSet[deriver].insert(firstSymbol);
            else if (isEpsilonProduction(rule))
                firstSet[deriver].insert(EPSILON);
        }
    }

    bool allContainEplision;
    int originalSize;
    bool originallyContainEpislon;
    bool changeOccur = true;
    while (changeOccur) {
        changeOccur = false;
        for (auto entry : grammar) {
            deriver = entry.first;
            productions = entry.second;

            originallyContainEpislon = containEpsilon(firstSet[deriver]);

            for (auto rule : productions) {
                allContainEplision = true;

                for (auto symbol : rule) {
                    if (isTerminal(symbol)) {
                        firstSet[deriver].insert(symbol);
                        allContainEplision = false;
                        break;
                    }
                    else if (isNonTerminal(symbol)) {
                        originalSize = firstSet[deriver].size();
                        firstSet[deriver] =
                            setUnion(firstSet[deriver], firstSet[symbol]);

                        // Remove EPSILON if firstSet[deriver] originally
                        // doesn't contain EPSILON
                        if (!originallyContainEpislon)
                            firstSet[deriver].erase(EPSILON);

                        if (firstSet[deriver].size() != originalSize)
                            changeOccur = true;

                        // If the first set of current symbol doesn't
                        // contains EPSILON, don't check the next symbol
                        if (!containEpsilon(firstSet[symbol])) {
                            allContainEplision = false;
                            break;
                        }
                    }
                }

                // If all productions deriverd from deriver contain EPSILON
                // add EPSILON to its firset
                originalSize = firstSet[deriver].size();
                if (allContainEplision)
                    firstSet[deriver].insert(EPSILON);
                if (firstSet[deriver].size() != originalSize)
                    changeOccur = true;
            }
        }
    }
}

void ParserGenerator::findFollowSet()
{
    // add $ to S's followSet
    followSet[START_SYMBOL].insert(END_SYMBOL);

    // for each non terminal A
    // follow(A) = {t | B->aAtW is a production}
    string symbol;
    string nextSymbol;
    for (auto entry : grammar)
        for (auto rule : entry.second)
            for (int i = 1; i < rule.size(); i++) {
                symbol = rule[i - 1];
                nextSymbol = rule[i];
                if (isTerminal(nextSymbol))
                    followSet[symbol].insert(nextSymbol);
            }

    int originalSize;
    string deriver;
    vector<vector<string> > productions;
    bool changeOccur = true;
    while (changeOccur) {
        changeOccur = false;
        for (auto entry : grammar) {
            deriver = entry.first;
            productions = entry.second;

            for (auto rule : productions) {
                if (rule.size() < 2) {
                    // If rule contain only one symbol
                    // union its follow set with deriver's
                    symbol = rule.front();
                    originalSize = followSet[symbol].size();
                    followSet[symbol] =
                        setUnion(followSet[symbol], followSet[deriver]);

                    if (originalSize != followSet[symbol].size())
                        changeOccur = true;
                }
                else {
                    // B->aAw
                    for (int i = 0; i < rule.size() - 1; i++) {
                        symbol = rule[i];
                        nextSymbol = rule[i + 1];

                        originalSize = followSet[symbol].size();

                        // follow(A) = follow(A)+first(w)-eplison
                        followSet[symbol] =
                            setUnion(excludeEpsilon(firstSet[nextSymbol]),
                                     followSet[symbol]);

                        // If EPSILON is in w, follow(A) = follow(A)+follow(B)
                        if (containEpsilon(firstSet[nextSymbol]))
                            followSet[symbol] =
                                setUnion(followSet[deriver], followSet[symbol]);

                        if (originalSize != followSet[symbol].size())
                            changeOccur = true;
                    }

                    // EPSILON is in w (this A is the last symbol)
                    // follow(A) = follow(A)+follow(B)
                    symbol = rule.back();
                    originalSize = followSet[symbol].size();
                    followSet[symbol] =
                        setUnion(followSet[symbol], followSet[deriver]);
                    if (originalSize != followSet[symbol].size())
                        changeOccur = true;
                }
            }
        }
    }
}

void ParserGenerator::exportSet(string fileName)
{
    ofstream outputFileStream(fileName);
    if (outputFileStream.is_open()) {
        outputFileStream << "First" << endl;
        for (auto i : inputSequence) {
            outputFileStream << left << setw(20) << i << ": ";
            for (auto j : firstSet[i])
                outputFileStream << j << " ";
            outputFileStream << endl;
        }

        outputFileStream << endl;

        outputFileStream << "Follow" << endl;
        for (auto i : inputSequence) {
            outputFileStream << left << setw(20) << i << ": ";
            for (auto j : followSet[i])
                outputFileStream << j << " ";
            outputFileStream << endl;
        }
        outputFileStream.close();
    }
    else {
        cout << "Cannot open file " << fileName << endl;
    }
}

bool ParserGenerator::containEpsilon(set<string> s)
{
    return s.find(EPSILON) != s.end();
}

set<string> ParserGenerator::excludeEpsilon(set<string> s)
{
    s.erase(EPSILON);
    return s;
}

bool ParserGenerator::isEpsilon(string word)
{
    return word == EPSILON;
}

bool ParserGenerator::isTerminal(string word)
{
    return terminals.find(word) != terminals.end();
}

bool ParserGenerator::isNonTerminal(string word)
{
    return nonTerminals.find(word) != nonTerminals.end();
}

map<string, map<string, vector<string> > > ParserGenerator::getParseTable()
{
    return parseTable;
}

set<string> ParserGenerator::getTerminals()
{
    return terminals;
}

set<string> ParserGenerator::getNonTerminals()
{
    return nonTerminals;
}

void ParserGenerator::printAll()
{
    cout << "----------These are non-terminals----------" << endl;
    for (auto i : nonTerminals)
        cout << i << endl;

    cout << "---------These are terminals----------" << endl;
    for (auto i : terminals)
        cout << i << endl;

    cout << "--------Rule---------" << endl;
    for (auto i : inputSequence) {
        cout << i << endl;
        for (auto j : grammar[i]) {
            cout << "\t";
            for (auto k : j)
                cout << k << " ";
            cout << endl;
        }
        cout << endl;
    }

    cout << "----------First Set---------" << endl;
    for (auto i : inputSequence) {
        cout << left << setw(20) << i << ": ";
        for (auto j : firstSet[i])
            cout << j << " ";
        cout << endl;
    }
    for (auto i : firstSet) {
        if (isTerminal(i.first)) {
            cout << left << setw(20) << i.first << ": ";
            for (auto j : i.second)
                cout << j << "  ";
            cout << endl;
        }
    }

    cout << "----------Follow Set---------" << endl;
    for (auto i : inputSequence) {
        cout << left << setw(20) << i << ": ";
        for (auto j : followSet[i])
            cout << j << " ";
        cout << endl;
    }
    for (auto i : followSet) {
        if (isTerminal(i.first)) {
            cout << left << setw(20) << i.first << ": ";
            for (auto j : i.second)
                cout << j << "  ";
            cout << endl;
        }
    }

    cout << "----------Parse Table---------" << endl;
    cout << START_SYMBOL << endl;
    for (auto i : inputSequence) {
        for (auto j : parseTable[i]) {
            cout << left << setw(20) << i << left << setw(10) << j.first;
            for (auto k : j.second) {
                cout << k << " ";
            }
            cout << endl;
        }
    }
}
const string ParserGenerator::EPSILON = "epsilon";
const string ParserGenerator::START_SYMBOL = "S";
const string ParserGenerator::END_SYMBOL = "$";
