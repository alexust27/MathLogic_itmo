#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include "include/parser.h"
#include <map>

using namespace std;

vector<Parser::_tree> forest;
vector<Parser::_tree> axioms;
unordered_map<string, string> axiomVer;
unordered_map<string, int> pr;

Parser parser;

void printer(int counter, const string &expr, int help, int x1, int x2) {
    cout << "(" << counter + 1 << ") ";
    cout << expr;
    if (help == 0)
        cout << " (Сх. акс. " << x1 << ")";
    else if (help == 1)
        cout << " (M.P. " << x1 + 1 << ", " << x2 + 1 << ")";
    else
        cout << " (Не доказано)";
    cout << endl;
}

bool checkAxiom(Parser::_tree vertex, Parser::_tree axiom) {
    if (!axiom && !vertex) {
        return true;
    } else if (!axiom || !vertex) {
        return false;
    }

    if (isalpha(axiom->sign[0])) {
        if (axiomVer.count(axiom->sign)) {
            return axiomVer[axiom->sign] == vertex->treeString;
        } else {
            axiomVer.insert({axiom->sign, vertex->treeString});
            return true;
        }
    }

    if (axiom->sign == vertex->sign) {
        return checkAxiom(vertex->left, axiom->left) &&
               checkAxiom(vertex->right, axiom->right);
    }

    return false;
}

bool isAxiom(Parser::_tree vertex, int counter, const string &cur_expr) {
    for (int i = 0; i < axioms.size(); i++) {
        axiomVer.clear();
        if (checkAxiom(vertex, axioms[i])) {
            printer(counter, cur_expr, 0, i + 1, -1);
            return true;
        }
    }
    return false;
}


int main() {

//    freopen("in.txt", "r", stdin);
//    freopen("out.txt", "w", stdout);

    axioms.push_back(parser.parse("A -> B -> A"));
    axioms.push_back(parser.parse("(A -> B) -> (A -> B -> C) -> (A -> C)"));
    axioms.push_back(parser.parse("A -> B -> A & B"));
    axioms.push_back(parser.parse("A & B -> A"));
    axioms.push_back(parser.parse("A & B -> B"));
    axioms.push_back(parser.parse("A -> A | B"));
    axioms.push_back(parser.parse("B -> A | B"));
    axioms.push_back(parser.parse("(A -> C) -> (B -> C) -> (A | B -> C)"));
    axioms.push_back(parser.parse("(A -> B) -> (A -> !B) -> !A"));
    axioms.push_back(parser.parse("!!A -> A"));

    string main_str;
    getline(cin, main_str);
    main_str = deleteWhitespace(main_str);
    unsigned int j = 0, k = 1;
    for (size_t i = 0; i < main_str.length(); ++i) {
        if (main_str[i] == ',' || (main_str[i] == '|' && main_str[i + 1] == '-')) {
            pr.insert({parser.parse(main_str.substr(j, i - j))->treeString, k++});
            j = i + 1;
        }
    }

    string current_str;
    for (int counter = 0; getline(cin, current_str); ++counter) {
        current_str = deleteWhitespace(current_str);
        if (current_str.length() == 0) {
            --counter;
            continue;
        }

        forest.push_back(parser.parse(current_str));
        if (pr.count(forest.back()->treeString)) {
            printf("(%d) ", counter + 1);
            cout << current_str;
            printf(" (Предп. %d)\n", pr[forest.back()->treeString]);
            continue;
        }

        if (!isAxiom(forest.back(), counter, current_str)) {

            Parser::_tree lastTree = forest.back();
            bool MP = false;
            for (int i = counter - 1; i >= 0 && !MP; --i) {
                if (forest[i]->right && forest[i]->sign == "->" && parser.equals(forest[i]->right, lastTree)) {
                    Parser::_tree leftTree = forest[i]->left;
                    for (int z = counter - 1; z >= 0; --z) {
                        if (parser.equals(forest[z], leftTree)) {
                            MP = true;
                            printer(counter, current_str, 1, i, z);
                            break;
                        }
                    }
                }
            }

            if (!MP) {
                printer(counter, current_str, 2, -1, -1);
                continue;
            }
        }
    }

    return 0;
}
