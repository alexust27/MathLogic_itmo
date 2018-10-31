#include <iostream>
#include <vector>
#include <map>
#include "parser.h"

using namespace std;

class Tree;

Parser parser;

typedef Tree *_world;
typedef Parser::_tree _term;

class Tree {

public:
    Tree(int n) {
        num = n;
    }

    Tree(int n, vector<string> var) {
        num = n;
        for (const auto &i : var) {
            vars.insert(i);
        }
    }

    int num;
    set<string> vars;
    vector<_world> childs;

    int kVer() {
        int k = 1;
        for (auto &child : childs)
            k += child->kVer();
        return k;
    }

    //подвешивает к вершине nVer вершину nAdd
    bool addVerToTree(int nVer, int nAdd) {
        bool find = false;
        if (num == nVer) {
            childs.push_back(new Tree(nAdd));
            find = true;
        } else {
            for (int i = 0; i < childs.size() && !find; ++i) {
                find = childs[i]->addVerToTree(nVer, nAdd);
            }
        }
        return find;
    }

    void print() {
        cout << num << ":\n";
        for (const auto &i : vars) {
            cout << i << endl;
        }
        for (auto &child : childs) {
            child->print();
        }
    }

};


bool dfs1(_world t, bool dad, const string &v) {
    if (dad && !t->vars.count(v)) {
        return false;
    }
    bool f = true;
    for (int i = 0; i < t->childs.size(); ++i) {
        f &= dfs1(t->childs[i], t->vars.count(v), v);
    }
    return f;
}

bool check1(_world w) {
    for (const auto &var : parser.vars) {
        if (!dfs1(w, false, var))
            return false;
    }
    return true;
}


bool getVal(_term cur, _world world);

bool applyBinOp(_term cur, bool x, bool y, _world world) {
    string s = cur->sign;
    if (s == "&") {
        return x & y;
    } else if (s == "|") {
        return x | y;
    } else if (s == "->") {
        bool f = (!x) | y;
        for (auto &child : world->childs) {
            f &= getVal(cur, child);
        }
        return f;
    } else {
        bool f = !x;
        for (auto &child : world->childs) {
            f &= getVal(cur, child);
        }
        return f;
    }
}

bool getVal(_term cur, _world world) {
    if (!cur) {
        return false;
    }

    if (isalpha(cur->sign[0])) {
        return (bool) world->vars.count(cur->sign);
    }

    bool leftVal = getVal(cur->left, world);
    bool rightVal = getVal(cur->right, world);

    return applyBinOp(cur, leftVal, rightVal, world);
}

bool check2(_world world, _term cur) {
    bool f = true;

    for (auto &child : world->childs) {
        f = f & check2(child, cur);
    }
    f = f & getVal(cur, world);
    return f;
}

void make_alg(vector<_world> &roots, size_t kV);

void check(const string &formula, vector<string> ss) {
    vector<_world> roots;
    _term tr = parser.parse(formula);
//    _world prev = nullptr;
//    int prevK = 0;
    map<int, _world> dad_byK;
    for (int i = 0; i < ss.size(); ++i) {
        string s = ss[i];
        int k = 0;
        //parse input string
        while (s[k] != '*') {
            k++;
        }
        vector<string> vars;
        string help;
        for (int j = k + 1; j < s.length(); ++j) {
            if (s[j] == ',') {
                vars.push_back(help);
                help.clear();
            } else {
                if (s[j] != ' ')
                    help.push_back(s[j]);
            }
        }
        if (!help.empty())
            vars.push_back(help);

        for (const auto &var : vars) {//add new var to parser.vars
            if (!parser.setVar.count(var)) {
                parser.vars.push_back(var);
                parser.setVar.insert(var);
            }
        }

        auto newWorld = new Tree(i, vars);
        if (k == 0 || roots.empty()) {
            roots.push_back(newWorld);
            dad_byK.clear();
            dad_byK[0] = roots.back();
        } else {
            dad_byK[k - 1]->childs.push_back(newWorld);
            dad_byK[k] = newWorld;
        }
    }

//    cout << roots.size();
    for (auto &root : roots) {
        if (!check1(root)) {
            cout << "Не модель Крипке";
            return;
        }
    }

    bool f = true;
    for (auto &root : roots) {
        f &= check2(root, tr);
    }
    if (f) {
        cout << "Не опровергает формулу";
        return;
    }

    make_alg(roots, ss.size());
}


map<int, _world> num_world;
vector<set<int>> baseT;
vector<set<int>> topology;

map<string, pair<int, int>> answer;

set<int> dfs3(_world w) {
    num_world.insert({w->num, w});
    set<int> trees;
    trees.insert(w->num);
    for (auto &child : w->childs) {
        set<int> s = dfs3(child);
        trees.insert(s.begin(), s.end());
    }
    baseT[w->num + 1] = trees;
    return trees;
}

struct Node {
    int num;
    vector<Node *> to;
};

vector<Node *> gr;

void make_alg(vector<_world> &roots, size_t kV) {
    kV++;
    baseT.resize(kV);
//    int offset = 0;
    for (auto &root : roots) {
        dfs3(root);//make base
//        offset += root->kVer();
    }
    baseT[0] = set<int>();

    for (int mask = 1; mask < (1 << kV); ++mask) {
        set<int> element;
        for (int i = 0; i < kV; ++i) {
            if ((1 << i) & mask) {
                element.insert(baseT[i].begin(), baseT[i].end());
            }
        }
        bool f = true;
        for (const auto &i : topology) {
            if (i == element) {
                f = false;
                break;
            }
        }
//        if(element.size() == 0) cout << topology.size();
        if (f) topology.push_back(element);
    }

    for (int i = 0; i < topology.size(); ++i) {
        gr.push_back(new Node{i});
    }

    for (int i = 0; i < topology.size(); ++i) {
        for (int j = 0; j < topology.size(); ++j) {
            if (i == j) {
                continue;
            }
            bool f = true;
            for (int e : topology[i]) {
                if (!topology[j].count(e)) {
                    f = false;
                    break;
                }
            }
            if (f) gr[i]->to.push_back(gr[j]);
        }
    }

    for (const auto &vv : parser.vars) {
        for (int i = 0; i < topology.size(); ++i) {
            int k = 0;
            for (int e : topology[i]) {
                auto wr = num_world[e];
                if (wr->vars.count(vv)) {
                    k++;
                } else {
                    k = -1;
                    break;
                }
            }
            if (!answer.count(vv) || answer[vv].first < k) {
                answer[vv].first = k;
                answer[vv].second = i;
            }
        }
        if (answer[vv].first == -1) {
            answer[vv].second = 0;
        }
    }

//    cout << "answer" << endl;
    cout << gr.size() << endl;
    for (int i = 0; i < gr.size(); ++i) {
        cout << gr[i]->num + 1 << ' ';
        for (int j = 0; j < gr[i]->to.size(); ++j) {
            cout << gr[i]->to[j]->num + 1 << ' ';
        }
        cout << endl;
    }

    for (int i = 0; i < parser.vars.size(); ++i) {
        string s = parser.vars[i];
        cout << s << "=" << answer[s].second + 1;
        if (i != parser.vars.size() - 1)
            cout << ",";
    }
}