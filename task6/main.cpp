#include <iostream>
#include "kripkeToGeyting.h"

using namespace std;

int main() {
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);

    Parser parser;

    string s;
    string formula;
    while (formula.length() == 0)
        getline(cin, formula);

    vector<string> ss;
    while (getline(cin, s)) {
        if (s.length())
            ss.push_back(s);
    }
    check(formula, ss);

    return 0;

}
