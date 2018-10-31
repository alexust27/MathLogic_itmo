#pragma once

#include <string>
#include <set>
#include <utility>
#include <vector>
#include <unordered_set>

using namespace std;

class Tree {
public:
    Tree(Tree *left, Tree *right, const string &sign) {
        this->left = left;
        this->right = right;
        this->sign = sign;
        if (left != nullptr) {
            this->freeVars.insert(left->freeVars.begin(), left->freeVars.end());
        }
        if (right != nullptr) {
            this->freeVars.insert(right->freeVars.begin(), right->freeVars.end());
        }
    }

//    Tree() = default;

    Tree(const string &sign, bool isVar) {
        this->sign = sign;
        if (isVar)
            this->freeVars.insert(sign);
    }

    ~Tree() {
        delete right;
        delete left;
    }

    string to_string() {
        if (sign == "->" || sign == "|" || sign == "&" || sign == "=" || sign == "+" || sign == "*") {
            return "(" + left->to_string() + sign + right->to_string() + ")";
        } else if (sign == "!") {
            return sign + left->to_string();
        } else if (sign == "\'") {
            return left->to_string() + "\'";
        } else if (sign == "?" || sign == "@") {
            return sign + left->to_string() + "(" + right->to_string() + ")";
        } else if (!terms.empty()) {
            string res = sign + "(";
            bool f = false;
            for (Tree *arg : terms) {
                if (f) res += ",";
                f = true;
                res += arg->to_string();
            }
            return res + ")";
        } else {
            return sign;
        }
    }

    string sign;
    Tree *left = nullptr;
    Tree *right = nullptr;
    vector<Tree *> terms;
    set<string> freeVars;

    static bool equals(Tree *first, Tree *second) {
        if (first == nullptr && second == nullptr) {
            return true;
        }
        if (first == nullptr || second == nullptr) {
            return false;
        }

        if (first->sign != second->sign)
            return false;
        if (first->terms.size() != second->terms.size())
            return false;
        for (int i = 0; i < first->terms.size(); ++i) {
            if (first->terms[i]->to_string() != second->terms[i]->to_string()) {
                return false;
            }
        }
        return equals(first->left, second->left) && equals(first->right, second->right);
    }

    unordered_set<string> get_vars() {
        unordered_set<string> res;
        if (islower(sign[0]) && terms.empty()) {
            res.insert(sign);
        }
        if (left != nullptr
//            && sign != "?" && sign != "@"
                ) {
            for (const string &s : left->get_vars()) {
                res.insert(s);
            }
        }
        if (right != nullptr) {
            for (const string &s : right->get_vars()) {
                res.insert(s);
            }
        }
        for (Tree *arg : terms) {
            for (const string &s : arg->get_vars()) {
                res.insert(s);
            }
        }
        return res;
    }
};

typedef Tree *_tree;
