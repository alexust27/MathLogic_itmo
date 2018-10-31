#include <iostream>
#include <algorithm>
#include "checker.h"

//task3

using namespace std;

void make_task() {
    Parser p;
    string s;
    vector<string> hyp;
    vector<string> forProof;

    freopen("input.txt", "r", stdin);
    getline(cin, s);
    s = deleteWhitespace(s);

    unsigned int j = 0;
    for (size_t i = 0; i < s.length(); ++i) {
        if (s[i] == ',') {
            hyp.push_back(p.parse(s.substr(j, i - j))->treeString);
            j = i + 1;
        }

        if (s[i] == '|' && s[i + 1] == '=') {
            if (i != 0) {
                hyp.push_back(p.parse(s.substr(j, i - j))->treeString);
            }
            j = i + 2;
            break;
        }
    }

    s = s.substr(j);
    forProof.push_back(s);
    for (int i = hyp.size() - 1; i >= 0; --i) {
        s = "(" + hyp[i] + ")->(" + s + ")";
        forProof.push_back(s);
    }
    if (forProof.size() > 1) {
        forProof.pop_back();
        reverse(forProof.begin(), forProof.end());
    }

    Checker ch;
    ch.check(p.parse(s)); 
    ch.printAnswer(hyp, forProof);
}

int main() {

    make_task();

    return 0;
}
