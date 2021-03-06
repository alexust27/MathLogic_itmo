#include <iostream>
#include <string>
#include <set>

using namespace std;

string deleteWhitespace(string str) {
    string new_str;
    for (char i : str) {
        if (!isspace(i) && i != '\t') {
            new_str.push_back(i);
        }
    }
    return new_str;
}

class Parser {
public:

    struct Tree {
        Tree() {
            left = right = nullptr;
        }

        ~Tree() {
            delete right;
            delete left;
        }

        Tree *right;
        Tree *left;
        string treeString, sign;
//        bool isVar = false;
    };

    typedef Tree *_tree;

    vector<string> vars;

    _tree parse(string s) {
        str = std::move(s);
        str = deleteWhitespace(str);
        index = 0;

        _tree tr = expr();
        for (string v : setVar) {
            vars.push_back(v);
        }
        return tr;
    }

    bool equals(Parser::Tree *first, Parser::Tree *second) {
        if (!first && !second) {
            return true;
        }
        if (!first || !second) {
            return false;
        }

        if (first->treeString != second->treeString) return false;
        return equals(first->left, second->left) && equals(first->right, second->right);
    }

    void correct(_tree tr) {
        if (tr == nullptr) return;
//        tr->isVar = tr->sign == tr->treeString;
        correct(tr->left);
        correct(tr->right);
    }

private:

    set<string> setVar;

    enum List {
        IMPL, OR, AND, NOT, bracket1, bracket2, var
    };
    List id;
    string str;
    string variable;

    int index;

    _tree expr() {
        _tree left = disjunction();
        _tree vertex = nullptr;

        while (true) {
            if (id == IMPL) {
                vertex = new Tree();
                vertex->left = left;
                vertex->right = expr();
                vertex->sign = "->";
                vertex->treeString = "(" + vertex->left->treeString + "->" + vertex->right->treeString + ")";
                left = vertex;
            } else {
                return left;
            }
        }
    }

    _tree disjunction() {
        _tree left = conjunction();
        _tree vertex = nullptr;

        while (true) {
            if (id == OR) {
                vertex = new Tree();
                vertex->left = left;
                vertex->right = conjunction();
                vertex->sign = "|";
                vertex->treeString = "(" + vertex->left->treeString + "|" + vertex->right->treeString + ")";
                left = vertex;
            } else {
                return left;
            }
        }

    }

    _tree conjunction() {
        _tree left = rez();
        _tree vertex = nullptr;

        while (true) {
            if (id == AND) {
                vertex = new Tree();
                vertex->left = left;
                vertex->right = rez();
                vertex->sign = "&";
                vertex->treeString = "(" + vertex->left->treeString + "&" + vertex->right->treeString + ")";
                left = vertex;
            } else {
                return left;
            }
        }
    }

    _tree rez() {
        getNext();
        _tree vertex = nullptr;
        switch (id) {
            case var:
                vertex = new Tree();
                vertex->sign = variable;
                vertex->treeString = variable;
//                vertex->isVar = true;
                getNext();
                break;
            case NOT:
                vertex = new Tree();
                vertex->left = rez();
                vertex->sign = "!";
                vertex->treeString = "(!" + vertex->left->treeString + ")";
                break;

            case bracket1:
                vertex = expr();
                if (id != bracket2) {
                    cout << ") missing";
                    exit(0);
                }
                getNext();
                break;

            default:
                cout << "Incorrect!";
                exit(0);
        }
        return vertex;
    }

    void getNext() {
        char ch = next();
        variable = "";
        if (isalpha(ch)) {
            while (isdigit(ch) || isalpha(ch)) {
                variable.push_back(ch);
                ch = next();
            }
            index--;
            id = var;
            setVar.insert(variable);
        } else {
            switch (ch) {
                case '!':
                    id = NOT;
                    break;
                case '&':
                    id = AND;
                    break;
                case '|':
                    id = OR;
                    break;
                case '-':
                    next();
                    id = IMPL;
                    break;
                case '(':
                    id = bracket1;
                    break;
                case ')':
                    id = bracket2;
                    break;
                default:
                    break;
            }
        }
    }

    char next() {
        if (index < str.length()) {
            return str[index++];
        } else {
            return '\0';
        }
    }
};
