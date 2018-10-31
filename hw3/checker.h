#pragma once
#include <iostream>
#include <string>
#include <stdexcept>
#include <cmath>
#include <fstream>
#include "deduction.h"

using namespace std;

class Checker {
    unordered_map<string, bool> badAnswer;
    vector<string> variables;
    unordered_map<char, vector<string>[4]> lemms;
    vector<string> aOrNota;
    ifstream file;
    vector<vector<string> > proof;
    Deduction deduction;

private:
    void initLemms(string path, int cnt, char binOp) {
        vector<string> temp[4];
        string s = "lems/" + path;
        int x, y, z;

        file.open(s.c_str(), ifstream::in);
        if (!file) {
            throw runtime_error("Can't open file " + s);
        }

        while (getline(file, s)) {
            s = deleteWhitespace(s);
            if (s.length() == 0) {
                continue;
            }
            if (s[0] == '=') {
                file >> x >> y >> z;
                z = (x << 1) + y;
                continue;
            }
            // cerr << s << " " << s.length() << endl;
            temp[z].push_back(s);
            s.clear();
        }

        // cerr << temp[0].size() << " ";

        for (int i = 0; i < cnt; ++i) {
            lemms[binOp][i] = temp[i];
        }
        file.close();
    }

    void init() {
        try {
            initLemms("not", 2, '!');
            initLemms("and", 4, '&');
            initLemms("or", 4, '|');
            initLemms("implication", 4, '-');

            file.open("lems/aOrNota", ifstream::in);
            string s;
            // Parser p;
            while (getline(file, s)) {
                s = deleteWhitespace(s);
                if (s.length() == 0) {
                    continue;
                }
                aOrNota.push_back(s);
            }
            // cerr << aOrNota.size() << ' ';
        } catch (...) {
            file.close();
            throw;
        }
        file.close();
    }

    bool applyBinOp(const string &s, bool x, bool y) {
        if (s == "&") {
            return x & y;
        } else if (s == "|")
            return x | y;
        else if (s == "->") {
            return !x | y;
        } else {
//        if (s == "!")
            return !x;
        }
    }

    void modifiedStrings(
            vector<string> const &from,
            vector<string> *to,
            string const &left,
            string const &right,
            int offset) {
        for (int i = 0; i < from.size(); ++i) {
            for (int j = 0; j < from[i].size(); ++j) {
                if (from[i][j] == 'A') {
                    (*to)[i + offset] += "(" + left + ")";
                } else if (from[i][j] == 'B') {
                    (*to)[i + offset] += "(" + right + ")";
                } else {
                    (*to)[i + offset].push_back(from[i][j]);
                }
            }
        }
    }

    void applyLem(char binOp, int offset, string const &left, string const &right) {
        vector<string> curLem = lemms[binOp][offset];
        int lst = proof.size() - 1;
        int lstProofSize = proof[lst].size();
        proof[lst].resize(lstProofSize + curLem.size());

        modifiedStrings(curLem, &proof[lst], left, right, lstProofSize);
    }

    int numOfVar;


    void merge(string result_str) {
        int lst = proof.size() - 2;

        deduction.solve(proof.back());
        proof.back() = deduction.result;

        deduction.solve(proof[lst]);
        proof[lst] = deduction.result;

        proof[lst].insert(proof[lst].end(), proof.back().begin() + 1, proof.back().end());
        proof.pop_back();

        string old_str = proof[lst][0];
        string new_first_str = "";
        for (size_t i = 0; i < old_str.length(); ++i) {
            if (old_str[i] == '|' && old_str[i + 1] == '-') {
                new_first_str += old_str.substr(0, i + 2);
                new_first_str += result_str;
                break;
            }
        }
        proof[lst][0] = new_first_str;

        int sizeBefore = proof[lst].size();
        string lastVarFromContext = variables[variables.size() - numOfVar];

        proof[lst].resize(sizeBefore + aOrNota.size());

        modifiedStrings(aOrNota, &proof[lst], lastVarFromContext, "q", sizeBefore);

        proof[lst].push_back(
                "((" + lastVarFromContext + ")->(" + result_str
                + "))->(!(" + lastVarFromContext + ")->(" + result_str
                + "))->(((" + lastVarFromContext + ")|(!(" + lastVarFromContext + ")))->(" + result_str + "))"
        );

        proof[lst].push_back(
                "(!(" + lastVarFromContext + ")->(" + result_str
                + "))->(((" + lastVarFromContext + ")|(!(" + lastVarFromContext + ")))->(" + result_str + "))"
        );

        proof[lst].push_back("((" + lastVarFromContext + ")|(!(" + lastVarFromContext + ")))->(" + result_str + ")");
        proof[lst].push_back(result_str);
    }

    bool getValue(Parser::_tree cur, bool checkOn) {
        if (!cur) {
            return false;
        }

        if (isalpha(cur->sign[0]) != 0) {
            return badAnswer[cur->sign];
        }

        bool leftVal = getValue(cur->left, checkOn);
        bool rightVal = getValue(cur->right, checkOn);
        if (!checkOn)
            if (cur->sign[0] == '!') {
                applyLem('!', leftVal, cur->left->treeString, "q");
            } else {
                applyLem(cur->sign[0], (leftVal << 1) + rightVal, cur->left->treeString, cur->right->treeString);
            }

        return applyBinOp(cur->sign, leftVal, rightVal);
    }


    bool checkOnFalse(Parser::_tree cur) {
        int sz_v = variables.size();
        for (int i = 0; i < (1 << sz_v); ++i) {
            for (int j = 0; j < sz_v; ++j) {
                badAnswer[variables[sz_v - j - 1]] = static_cast<bool>(i & (1 << j));
            }
            bool f = getValue(cur, true);
            if (!f)
                return false;
        }

        for (int i = 0; i < (1 << sz_v); ++i) {
            string context = "";

            for (int j = 0; j < sz_v; ++j) {
                badAnswer[variables[sz_v - j - 1]] = static_cast<bool>(i & (1 << j));
            }

            for (int j = 0; j < sz_v; ++j) {
                if (!badAnswer[variables[j]]) {
                    context += "!";
                }

                context += variables[j];
                if (j != sz_v - 1) {
                    context += ",";
                } else {
                    context += "|-";
                }
            }
            context += cur->treeString;
            proof.push_back(vector<string>(1));
            proof.back()[0] = context;
            getValue(cur, false);

            numOfVar = 1;
            for (int j = 2; ((i + 1) & (j - 1)) == 0; j <<= 1) {
                merge(cur->treeString);
                ++numOfVar;
            }
        }
        return true;
    }

    void getVar(Parser::_tree cur) {
        if (!cur) return;

        if (!cur->left && !cur->right) {
            if (badAnswer.count(cur->sign) == 0) {
                badAnswer.insert({cur->sign, 0});
                variables.push_back(cur->sign);
            }
            return;
        }
        getVar(cur->left);
        getVar(cur->right);
    }

    bool fff;
public:

    void check(Parser::_tree cur) {
        init();
        getVar(cur);
        fff = false;
        fff = checkOnFalse(cur);
    }

    void printAnswer(vector<string> hyp, vector<string> hypHelp) {

        freopen("output.txt", "w", stdout);

        if (!fff) {
            cout << "Высказывание ложно при ";
            for (int i = 0; i < variables.size(); ++i) {
                string it = variables[i];
                cout << it << '=' << (badAnswer[it] ? "И" : "Л");

                if (i != variables.size() - 1)
                    cout << ", ";
            }
        } else {
            proof[0][0] = "";
            for (int i = 0; i < hyp.size(); ++i) {
                proof[0].push_back(hyp[i]);
                proof[0].push_back(hypHelp[i]);
                proof[0][0] += hyp[i];
                if (i != hyp.size() - 1)
                    proof[0][0].push_back(',');
            }
            proof[0][0] += "|-" + hypHelp.back();

            for (int i = 0; i < proof[0].size(); ++i) {
                cout << proof[0][i];
                if (i != proof[0].size() - 1)
                    cout << endl;
            }
        }
    }
};
