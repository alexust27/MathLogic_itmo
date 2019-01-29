#include <iostream>
#include "kripke.h"

using namespace std;

int main() {
//    freopen("input.txt", "r", stdin);
//    freopen("output.txt", "w", stdout);

    string s = "";
    while (s.length() == 0)
        getline(cin, s);
//    cout << s;
//    s = deleteWhitespace(s);
    task(s);

    return 0;
}