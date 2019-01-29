#include <iostream>
#include "include/solve.h"

using namespace std;

int main() {
//    freopen("in.txt", "r", stdin);
//    freopen("out.txt", "w", stdout);
    Parser p;
    Solve A;
    string s;
    vector<string> expressions;
    while (getline(cin, s)) {
        if (s.length() == 0)
            continue;
        if (expressions.empty())
            expressions.push_back(deleteWhitespace(s));
        else
            expressions.push_back("(" + deleteWhitespace(s) + ")");
    }

    A.solve(expressions);
    for (const auto &i : A.result) {
        cout << i << endl;
    }

    return 0;
}
