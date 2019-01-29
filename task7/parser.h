#pragma once

#include <iostream>
#include "tree.h"


string deleteWhitespace(string str) {
    string new_str;
    for (char i : str) {
        if (!isspace(i) && i != 13 && i != '\n') {
            new_str.push_back(i);
        }
    }
    return new_str;
}


class Parser {
public:


    _tree parse(string s) {
        str = deleteWhitespace(s);
        if (str.empty()) {
            cerr << "Empty string parse";
            exit(0);
        }
        index = 0;

        return expr();
    }


private:

    enum List {
        IMPL, OR, AND, NOT, A, Q, PRED, func, prime, sPRED,
        plus, mul, zero, comma, equal, bracket1, bracket2, var, END, num
    };
    List id;
    string str;
    string variable;

    int index;


    _tree expr() {
        _tree left = disjunction();

        while (true) {
            if (id == IMPL) {
                left = new Tree(left, expr(), "->");
            } else {
                return left;
            }
        }
    }

    _tree disjunction() {
        _tree left = conjunction();

        while (true) {
            if (id == OR) {
                left = new Tree(left, conjunction(), "|");
            } else {
                return left;
            }
        }

    }

    _tree conjunction() {
        _tree left = unar();

        while (true) {
            if (id == AND) {
                left = new Tree(left, unar(), "&");
            } else {
                return left;
            }
        }
    }

    _tree unar() {
        getNext();
        _tree vertex = nullptr;
        switch (id) {

            case NOT:
                vertex = new Tree(unar(), nullptr, "!");
                break;

            case A:
                vertex = new Tree(varr(), unar(), "@");
                vertex->freeVars.erase(vertex->left->sign);
                break;

            case Q:
                vertex = new Tree(varr(), unar(), "?");
                vertex->freeVars.erase(vertex->left->sign);
                break;

            case bracket1: {
                int i = index;
                bool isTerm = true;
                for (int balance = 1; balance != 0; ++i) {
                    if (str[i] == '(')
                        balance++;
                    if (str[i] == ')')
                        balance--;
                    if (str[i] == '-' && str[i + 1] == '>' || str[i] == '&' || str[i] == '|' || str[i] == '@' ||
                        str[i] == '?' || str[i] == '!' || isBigLet(str[i]) || str[i] == '=')
                        isTerm = false;
                }

                if (!isTerm) {
                    vertex = expr();
                    if (id != bracket2) {
                        cout << index << ":" << str[index];
                        cout << ") missing unar";
                        exit(0);
                    }
                    getNext();
                } else {
                    vertex = pred();
                }

                break;
            }
            case num://for axiom
                vertex = new Tree(nullptr, nullptr, variable);
                getNext();
                break;
            case sPRED:
                vertex = new Tree(nullptr, nullptr, variable);
                getNext();
                break;
            default:
                vertex = pred();
                break;
        }
        return vertex;
    }

    _tree varr() {
        getNext();
        auto ver = new Tree(variable, false);
        return ver;
    }


    _tree pred() {
        _tree vertex = nullptr;
        if (id == PRED) {
            vertex = new Tree(variable, false);
            vertex->terms.push_back(term());
            vertex->freeVars.insert(
                    vertex->terms.back()->freeVars.begin(),
                    vertex->terms.back()->freeVars.end());
            while (id == comma) {
                vertex->terms.push_back(term());
                vertex->freeVars.insert(
                        vertex->terms.back()->freeVars.begin(),
                        vertex->terms.back()->freeVars.end());
            }
            getNext();
        } else {
            getPrev();
            vertex = term();
            vertex = new Tree(vertex, term(), "=");
        }
        return vertex;
    }

    _tree term() {
        _tree left = sum();

        while (true) {
            if (id == plus) {
                left = new Tree(left, sum(), "+");
            } else {
                return left;
            }
        }
    }

    _tree sum() {
        _tree left = mult();

        while (true) {
            if (id == mul) {
                left = new Tree(left, mult(), "*");
            } else {
                return left;
            }
        }
    }

    _tree mult() {
        getNext();
        _tree vertex = nullptr;
        switch (id) {
            case func:
                vertex = new Tree(variable, false);
                vertex->terms.push_back(term());
                vertex->freeVars.insert(
                        vertex->terms.back()->freeVars.begin(),
                        vertex->terms.back()->freeVars.end());
                while (id == comma) {
                    vertex->terms.push_back(term());
                    vertex->freeVars.insert(
                            vertex->terms.back()->freeVars.begin(),
                            vertex->terms.back()->freeVars.end());
                }
                break;
            case var:
                vertex = new Tree(variable, true);
                break;
            case bracket1:
                vertex = term();
                if (id != bracket2) {
//                    cout << str << endl;
//                    cout << index << endl;
//                    cout << str[index - 1] << ' ' << str[index + 1];
                    cerr << ") missing";
                    exit(0);
                }
                break;
            case zero:
                vertex = new Tree("0", false);
                break;
            default:
                cerr << "mult " << str[index] << ":" << index << endl;
                break;
        }
        getNext();
        while (id == prime) {
            vertex = new Tree(vertex, nullptr, "'");
            getNext();
        }
        return vertex;
    }

    bool isBigLet(char ch) {
        return (ch >= 'A' && ch <= 'Z');
    }

    void getNext() {
        char ch = next();
        variable = "";
        if (isalpha(ch)) {
            if (!isBigLet(ch)) {
                while (isdigit(ch) || isalpha(ch)) {
                    if (isBigLet(ch))
                        break;
                    variable.push_back(ch);
                    ch = next();
                }
                if (id != A && id != Q && ch == '(') {
                    id = func;
                } else {
                    id = var;
                    index--;
                }
            } else {
                while (isdigit(ch) || isalpha(ch)) {
                    variable.push_back(ch);
                    ch = next();
                }
                if (ch == '(')
                    id = PRED;
                else {
                    --index;
                    id = sPRED;
                }
            }
        } else if (isdigit(ch) && ch != '0') {
            variable.push_back(ch);
            id = num;
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
                    index++;
                    id = IMPL;
                    break;
                case '(':
                    id = bracket1;
                    break;
                case ')':
                    id = bracket2;
                    break;
                case '@':
                    id = A;
                    break;
                case '?':
                    id = Q;
                    break;
                case ',':
                    id = comma;
                    break;
                case '=':
                    id = equal;
                    break;
                case '+':
                    id = plus;
                    break;
                case '*':
                    id = mul;
                    break;
                case '0':
                    id = zero;
                    break;
                case '\'':
                    id = prime;
                    break;
                case '#':
                    id = END;
                    break;
                default:
                    cerr << "incorrect character: " << ch << " at index: " << index << endl;
                    exit(0);
            }
        }
    }

    void getPrev() {
        if (id == func || id == var) {
            index -= variable.length();
        } else {
            index--;
        }
    }

    char next() {
        if (index < str.length()) {
            return str[index++];
        } else {
            return '#';
        }
    }
};


