#pragma once
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "parser.h"

using namespace std;

class Deduction {

private:

    vector<Parser::_tree> trees;
    vector<Parser::_tree> axioms;
    unordered_map<string, string> axiomVer;
    unordered_set<string> supp;

    Parser parser;

    bool checkAxiom(Parser::_tree vertex, Parser::_tree axiom) {
        if ((!axiom) && (!vertex)) {
            return true;
        } else if ((!axiom) || (!vertex)) {
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

    bool isAxiom(Parser::_tree vertex) {
        for (int i = 0; i < axioms.size(); ++i) {
            axiomVer.clear();
            if (checkAxiom(vertex, axioms[i])) {
                return true;
            }
        }
        return false;
    }

    void init_axioms() {
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
    }

    void makeMP(const string &A, const string &B, const string &C) {
        string expr;
        //(A -> B) -> ((A -> (B -> C)) -> (A -> C))
        expr = "((" + A + ")->(" + B + "))->(((" + A + ")->((" + B + ")->(" + C + ")))->((" + A + ")->(" + C + ")))";

//        result.push_back("MP");

        result.push_back(expr);
        //(A -> (B -> C)) -> (A -> C)
        expr = "((" + A + ")->((" + B + ")->(" + C + ")))->((" + A + ")->(" + C + "))";

        result.push_back(expr);
        expr = "((" + A + ")->(" + C + "))";

        result.push_back(expr);
    }

    void makeifEqual(const string &A) {
        // ax1
        string expr;
        string aa = "((" + A + ")->(" + A + "))";
        expr = "(" + A + ")->" + aa;

//        result.push_back("equal");

        result.push_back(expr);

        //ax 2
        //(a->(a->a))->(a->((a->a)->a))->(a->a)
        expr = "((" + A + ")->" + aa + ")->((" + A + ")->(" + aa + "->(" + A + ")))->" + aa;

        result.push_back(expr);

        //MP
        //(a->((a->a)->a))->(a->a)
        expr = "((" + A + ")->(" + aa + "->(" + A + ")))->" + aa;

        result.push_back(expr);

        // ax 1
        expr = "((" + A + ")->((" + aa + ")->(" + A + ")))";

        result.push_back(expr);

        //MP
        //expr = A + "->" + A;
        result.push_back(aa);
    }

    string make_first_str(string first_str) {

        string main_str = first_str;
        unsigned int j = 0, k = 2;
        string last_supp;
        for (size_t i = 0; i < main_str.length(); ++i) {
            if (main_str[i] == ',') {
                supp.insert(parser.parse(main_str.substr(j, i - j))->treeString);
                j = i + 1;
            }

            if (main_str[i] == '|' && main_str[i + 1] == '-') {
                last_supp = parser.parse(main_str.substr(j, i - j))->treeString;
                k = i + 2;
                break;
            }
        }

        string first_new;
        if (j > 0)
            first_new += main_str.substr(0, j - 1);
        first_new += "|-(" + last_supp + ")->(" + main_str.substr(k) + ")";

        result.push_back(first_new);
        return last_supp;
    }


public:

    vector<string> result;

    Deduction() {
        init_axioms();
    }

//    Parser::_tree axiom_n(int i){
//        return axioms[i];
//    }

    void solve(vector<string> expressions) {

        result.clear();
        trees.clear();
        supp.clear();
        axiomVer.clear();


        string last_supp = make_first_str(expressions[0]);

        // cerr << expressions.size() << endl;
        for (int counter = 0; counter < expressions.size() - 1; ++counter) {
            string cur_expr = deleteWhitespace(expressions[counter + 1]);
            if (cur_expr.length() == 0){
                  
                continue;
            } 
            // cerr << cur_expr << endl;
            trees.push_back(parser.parse(cur_expr));
            string current_str = trees.back()->treeString;

            if (supp.count(current_str) || isAxiom(trees.back())) {
                result.push_back(current_str);
                string expr;
                expr = "(" + current_str + ")->((" + last_supp + ")->(" + current_str + "))";
                result.push_back(expr);
                expr = "(" + last_supp + ")->(" + current_str + ")";
                result.push_back(expr);
            } else {
                Parser::_tree lastTree = trees.back();
                bool MP = false;
                for (int i = counter - 1; i >= 0 && !MP; --i)
                    if (trees[i]->right && trees[i]->sign == "->" && parser.equals(trees[i]->right, lastTree)) {
                        Parser::_tree leftTree = trees[i]->left;
                        for (int z = counter - 1; z >= 0; --z)
                            if (parser.equals(trees[z], leftTree)) {
                                MP = true;
                                makeMP(last_supp, expressions[z + 1], current_str);
                                break;
                            }
                    }
                if (!MP) {
                    makeifEqual(last_supp);
                }
            }
        }
    }
};
