#include <iostream>
#include <algorithm>
#include "checker.h"

void parse() {
    Parser p;
    string s;
    int i = 0;
    while (getline(cin, s)) {
//        cerr << ++i;
//    getline(cin, s);

        if (s == "=")
            break;
        auto tr = p.parse(s);
        cout << tr->to_string() << endl;
    }
}

int main() {

    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);

//    parse();
    task();

    return 0;
}
