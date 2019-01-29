#include <iostream>
#include <vector>
#include <map>
#include "parser.h"

using namespace std;

class Tree;

typedef Tree *_world;
typedef Parser::_tree _term;

class Tree {

public:
    Tree(int n) {
        num = n;
    }

    int num;
    map<string, bool> vars;
    vector<_world> childs;

    int kVer() {
        int k = 1;
        for (int i = 0; i < childs.size(); ++i)
            k += childs[i]->kVer();
        return k;
    }

    //подвешивает вершину к вершине
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
        for (auto i : vars) {
            cout << i.first << '=' << i.second << endl;
        }
        for (int i = 0; i < childs.size(); ++i) {
            childs[i]->print();
        }
//        cout << num << endl;
    }

};


map<int, vector<_world>> trees;

_world copyTree(_world t) {
    auto newTree = new Tree(t->num);
    for (int i = 0; i < t->childs.size(); ++i) {
        newTree->childs.push_back(copyTree(t->childs[i]));
    }
    return newTree;
}


void dfs(_world t, int mask, bool dad, string v) {
    t->vars[v] = dad ? true : ((bool) (mask & (1 << t->num)));
    for (int i = 0; i < t->childs.size(); ++i) {
        dfs(t->childs[i], mask, t->vars[v], v);
    }
}

bool getVal(_term cur, _world world);

void make_alg(_world root, int kV);

bool dfs2(_world world, _term cur);

bool applyBinOp(_term cur, bool x, bool y, _world world) {
    string s = cur->sign;
    if (s == "&") {
        return x & y;
    } else if (s == "|") {
        return x | y;
    } else if (s == "->") {
        bool f = (!x) | y;
        for (int i = 0; i < world->childs.size(); ++i) {
            f &= getVal(cur, world->childs[i]);
        }
        return f;
    } else {
        bool f = !x;
        for (int i = 0; i < world->childs.size(); ++i) {
            f &= getVal(cur, world->childs[i]);
        }
        return f;
    }
}

bool getVal(_term cur, _world world) {
    if (!cur) {
        return false;
    }

    if (isalpha(cur->sign[0])) {
        return world->vars.at(cur->sign);
    }

    bool leftVal = getVal(cur->left, world);
    bool rightVal = getVal(cur->right, world);

    return applyBinOp(cur, leftVal, rightVal, world);
}


bool dfs2(_world world, _term cur) {
    bool f = true;

    for (int i = 0; i < world->childs.size(); ++i) {
        f = f & dfs2(world->childs[i], cur);
//        f = f & getVal(cur, world->childs[i]);
    }
    f = f & getVal(cur, world);
    return f;
}

Parser parser;

void task(string s) {

    trees[1].push_back(new Tree(0));
    for (int nVertex = 2; nVertex <= 5; ++nVertex) {
        for (int i = 0; i < trees[nVertex - 1].size(); ++i) {
            auto root = trees[nVertex - 1][i];
            int kolV = root->kVer();
            for (int k = 0; k < kolV; ++k) {
                auto newTree = copyTree(root);
                newTree->addVerToTree(k, kolV);
                trees[nVertex].push_back(newTree);
            }
        }
    }
    cout << trees[5].size() << " - size\n";
    _term tt = parser.parse(s);
    for (int kVer = 1; kVer < 7; ++kVer) {
        for (int j = 0; j < trees[kVer].size(); ++j) {
            auto root = trees[kVer][j];
            for (int mask = 0; mask < (1 << (kVer * parser.vars.size())); ++mask) {
                for (int v = 0; v < parser.vars.size(); ++v) {
                    dfs(root, mask >> (v * kVer), false, parser.vars[v]);
                }
                bool f = dfs2(root, tt);
                if (!f) {
//                    cout << kVer << endl;
//                    root->print();
//                    cout << "stop" << mask;
                    make_alg(root, kVer);
                    return;
                }
            }
        }
    }
    cout << "Формула общезначима";
}

map<int, _world> num_world;
vector<set<int>> baseT;
vector<set<int>> topology;

map<string, pair<int, int>> answer;

set<int> dfs3(_world w) {
    num_world.insert({w->num, w});
    set<int> trees;
    trees.insert(w->num);
    for (int i = 0; i < w->childs.size(); ++i) {
        set<int> s = dfs3(w->childs[i]);
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

void make_alg(_world root, int kV) {
    kV++;
    baseT.resize(kV);
    baseT[0] = set<int>();

    dfs3(root);//make base

    for (int mask = 1; mask < (1 << kV); ++mask) {
        set<int> element;
        for (int i = 0; i < kV; ++i) {
            if ((1 << i) & mask) {
                element.insert(baseT[i].begin(), baseT[i].end());
            }
        }
        bool f = true;
        for (int i = 0; i < topology.size(); ++i) {
            if (topology[i] == element) {
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

    for (auto vv : parser.vars) {
        for (int i = 0; i < topology.size(); ++i) {
            int k = 0;
            for (int e : topology[i]) {
                auto wr = num_world[e];
                if (wr->vars[vv]) {
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