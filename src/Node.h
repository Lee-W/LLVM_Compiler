#ifndef NODE_H
#define NODE_H

#include <iostream>
using namespace std;

struct Node {
    Node() {}

    Node(int la, string s)
    {
        layer = la;
        symbol = s;
    }

    int layer;
    string symbol;
};

#endif
