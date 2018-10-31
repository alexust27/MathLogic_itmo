#include "parser.h"
#include <unordered_map>
#include <unordered_set>

Parser parser;

vector<_tree> logic_axioms;
vector<_tree> fa_axioms;
vector<_tree> supp;
set<string> free_vars;
string error;
vector<_tree> proved;
unordered_map<string, string> axiomVer;
unordered_map<string, _tree> vars;
_tree last_sup;

vector<_tree> parse_header(string header) {

    vector<_tree> res;
    int balance = 0;
    string tmp;
    for (int i = 0; i < header.length(); ++i) {
        if (header[i] == ',' && balance == 0 || header[i] == '|' && header[i + 1] == '-') {
            if (!tmp.empty()) {
                res.push_back(parser.parse(tmp));
            }
            if (header[i] == '|' && header[i + 1] == '-') {
                ++i;
            }
            tmp = "";
        } else {
            if (header[i] == '(') {
                ++balance;
            }
            if (header[i] == ')') {
                --balance;
            }
            tmp += header[i];
        }
    }
    res.push_back(parser.parse(tmp));
    return res;
}

void init_axioms() {
    logic_axioms.push_back(parser.parse("1->2->1"));
    logic_axioms.push_back(parser.parse("(1->2)->(1->2->3)->(1->3)"));
    logic_axioms.push_back(parser.parse("1->2->(1&2)"));
    logic_axioms.push_back(parser.parse("1&2->1"));
    logic_axioms.push_back(parser.parse("1&2->2"));
    logic_axioms.push_back(parser.parse("1->(1|2)"));
    logic_axioms.push_back(parser.parse("2->(1|2)"));
    logic_axioms.push_back(parser.parse("(1->3)->(2->3)->((1|2)->3)"));
    logic_axioms.push_back(parser.parse("(1->2)->(1->!2)->!1"));
    logic_axioms.push_back(parser.parse("!!1->1"));

    fa_axioms.push_back(parser.parse("a=b->a'=b'"));
    fa_axioms.push_back(parser.parse("a=b->a=c->b=c"));
    fa_axioms.push_back(parser.parse("a'=b'->a=b"));
    fa_axioms.push_back(parser.parse("!a'=0"));
    fa_axioms.push_back(parser.parse("a+b'=(a+b)'"));
    fa_axioms.push_back(parser.parse("a+0=a"));
    fa_axioms.push_back(parser.parse("a*0=0"));
    fa_axioms.push_back(parser.parse("a*b'=a*b+a"));
}

bool checkAxiom(_tree vertex, _tree axiom) {
    if (axiom == nullptr && vertex == nullptr) {
        return true;
    } else if (axiom == nullptr || vertex == nullptr) {
        return false;
    }

    if (isdigit(axiom->sign[0])) {
        if (axiomVer.count(axiom->sign)) {
            return axiomVer[axiom->sign] == vertex->to_string();
        } else {
            axiomVer.insert({axiom->sign, vertex->to_string()});
            return true;
        }
    }

    if (axiom->sign == vertex->sign) {
        return checkAxiom(vertex->left, axiom->left) &&
               checkAxiom(vertex->right, axiom->right);
    }

    return false;
}

bool isAxiom(_tree vertex) {
    for (int i = 0; i < logic_axioms.size(); ++i) {
        axiomVer.clear();
        if (checkAxiom(vertex, logic_axioms[i])) {
            return true;
        }
    }

    for (int i = 0; i < fa_axioms.size(); ++i) {
        if (fa_axioms[i]->to_string() == vertex->to_string())
            return true;
    }
    return false;
}

Tree *substitute(Tree *expr, const string &var, Tree *theta) {
    if (expr == nullptr) return expr;
    if (expr->terms.empty() && expr->sign == var) {
        return theta;
    }
    return new Tree(substitute(expr->left, var, theta), substitute(expr->right, var, theta), expr->sign);
}

bool check_on_substitution(_tree axiom, _tree expr) {
    if (expr == nullptr) return false;
    if (axiom->sign == "->" || axiom->sign == "|" || axiom->sign == "&"
        || axiom->sign == "=" || axiom->sign == "+" || axiom->sign == "*"
        || axiom->sign == "@" || axiom->sign == "?") {
        if (axiom->sign != expr->sign) {
            return false;
        }
        return check_on_substitution(axiom->left, expr->left)
               && check_on_substitution(axiom->right, expr->right);
    }
    if (axiom->sign == "!" || axiom->sign == "\'") {
        if (axiom->sign != expr->sign) {
            return false;
        }
        return check_on_substitution(axiom->left, expr->left);
    }
    if (!axiom->terms.empty()) {
        if (axiom->sign != expr->sign || axiom->terms.size() != expr->terms.size()) {
            return false;
        }
        for (int i = 0; i < axiom->terms.size(); ++i) {
            if (!check_on_substitution(axiom->terms[i], expr->terms[i])) {
                return false;
            }
        }
        return true;
    }
    if (vars.count(axiom->sign) == 0) {
        vars[axiom->sign] = expr;
    }
    return Tree::equals(vars[axiom->sign], expr);
}

// ψ[x := 0] & ∀x((ψ) → (ψ)[x := x']) → ψ
bool check_induction(_tree expr) {
    if (expr->sign == "->" &&
        expr->left->sign == "&" &&
        expr->left->right->sign == "@" &&
        expr->left->right->right->sign == "->") {
        _tree r = expr->right;
        string x = expr->left->right->left->sign;

        auto tmp = new Tree(
                new Tree(
                        substitute(r, x, new Tree("0", false)),
                        new Tree(
                                new Tree(x, true),
                                new Tree(r,
                                         substitute(r, x, new Tree(new Tree(x, true), nullptr, "\'")),
                                         "->"),
                                "@"),
                        "&"),
                r,
                "->");
        return Tree::equals(expr, tmp);
    }
    return false;
}

// axiom 11 ∀x(ψ)->(ψ[x := θ])
bool check_11(_tree expr) {
    if (expr->sign == "->" && expr->left->sign == "@") {
        string x = expr->left->left->sign;
        vars.clear();
        _tree l = expr->left;
        _tree r = expr->right;

        if (!check_on_substitution(l->right, r)) {
            return false;
        }

        if (r->get_vars().count(x) && vars[x]->get_vars().count(x) == 0) {
            return false;
        }

        unordered_set<string> fi_variables = l->right->get_vars();

        unordered_map<string, _tree> vv;
        for (auto qq : vars) {
            if (qq.first != qq.second->to_string() || qq.first == x) {
                vv.insert(qq);
            }
        }

        if (vv.empty() && fi_variables.count(x) == 0)
            return true;

        if (vv.size() == fi_variables.count(x) && vv.count(x)) {
            _tree theta = vv[x];
            unordered_set<string> fi_free_variables;
            fi_free_variables.insert(r->freeVars.begin(), r->freeVars.end());
            for (const string &s : theta->freeVars) {
                if (fi_free_variables.count(s) == 0) {
                    error = "терм " + theta->to_string()
                            + " не свободен для подстановки в формулу "
                            + l->right->to_string() + " вместо переменной " + x;
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

// axiom 12 (ψ[x := θ]) → ∃x(ψ)
bool check_12(_tree expr) {
    if (expr->sign == "->" && expr->right->sign == "?") {
        string x = expr->right->left->sign;
        _tree r = expr->right;
        _tree l = expr->left;
        vars.clear();

        if (!check_on_substitution(r->right, l)) {
            return false;
        }
        if (l->get_vars().count(x) && vars[x]->get_vars().count(x) == 0) {
            return false;
        }
        unordered_map<string, _tree> vv;
        unordered_set<string> fi_variables = r->right->get_vars();

        for (auto qq : vars) {
            if (qq.first != qq.second->to_string() || qq.first == x) {
                vv.insert(qq);
            }
        }

        if (vv.empty() && fi_variables.count(x) == 0) {
            return true;
        }
        if (vv.size() == fi_variables.count(x) && vv.count(x)) {
            auto theta = vv[x];
            unordered_set<string> fi_free_variables;
            fi_free_variables.insert(l->freeVars.begin(), l->freeVars.end());
            for (const string &s : theta->freeVars) {
                if (fi_free_variables.count(s) == 0) {
                    error = "терм " + theta->to_string()
                            + " не свободен для подстановки в формулу "
                            + r->right->to_string() + " вместо переменной " + x;
                    return false;
                }
            }
            return true;
        }

    }
    return false;
}

// rule for (φ) → ∀x(ψ)
bool rule1(_tree expr) {
    if (expr->sign == "->" && expr->right->sign == "@") {
        string x = expr->right->left->to_string();
        if (expr->left->freeVars.count(x)) {
            error = "переменная " + expr->right->left->to_string()
                    + " входит свободно в формулу " + expr->left->to_string();
            return false;
        }
//        if (last_sup->freeVars.count(x)) {
//            error = "используется правило с квантором по переменной " + x + ", входящей свободно в допущение " +
//                    last_sup->to_string();
//            return false;
//        }
        for (int i = (int) proved.size() - 1; i >= 0; --i) {
            if (proved[i]->sign != "->")
                continue;
            if (proved[i]->left->to_string() == expr->left->to_string() &&
                proved[i]->right->to_string() == expr->right->right->to_string()) {
                return true;
            }
        }
    }
    return false;
}

// rule for ∃x(ψ) → (φ)
bool rule2(_tree expr) {

    if (expr->sign == "->" && expr->left->sign == "?") {
        string x = expr->left->left->to_string();
        if (expr->right->freeVars.count(x)) { // x is free in φ
            error = "переменная " + x
                    + " входит свободно в формулу " + expr->right->to_string();
            return false;
        }
//        if (last_sup->freeVars.count(x)) {
//            error = "используется правило с квантором по переменной " + x + ", входящей свободно в допущение " +
//                    last_sup->to_string();
//            return false;
//        }
        for (int i = (int) proved.size() - 1; i >= 0; --i) {
            if (proved[i]->sign != "->")
                continue;
            if (proved[i]->left->to_string() == expr->left->right->to_string() &&
                proved[i]->right->to_string() == expr->right->to_string())
                return true;
        }
    }
    return false;

}

bool check(_tree expr) {
    for (_tree pr : supp) {
        if (pr->to_string() == expr->to_string()) {
            return true;
        }
    }
    if (isAxiom(expr) || check_induction(expr) || check_11(expr) || check_12(expr))
        return true;

    bool MP = false;
    int counter = (int) proved.size() - 1;
    for (int i = counter; i >= 0 && !MP; --i) {
        if (proved[i]->right && proved[i]->sign == "->" && Tree::equals(proved[i]->right, expr)) {
            _tree leftTree = proved[i]->left;
            for (int z = counter - 1; z >= 0; --z) {
                if (Tree::equals(proved[z], leftTree)) {
                    MP = true;
                    break;
                }
            }
        }
    }
    if (MP)
        return true;

    if (rule1(expr) || rule2(expr))
        return true;

}

void task() {
    init_axioms();
    string s;
    getline(cin, s);
    supp = parse_header(deleteWhitespace(s));
//    _tree main_expr = supp.back();
    supp.pop_back();
    last_sup = supp.back();
    for (_tree expr : supp) {
        free_vars.insert(expr->freeVars.begin(), expr->freeVars.end());
    }
    int line = 0;
    while (getline(cin, s)) {
        s = deleteWhitespace(s);
        if (s.empty()) continue;
        ++line;
        _tree cur = parser.parse(s);
        proved.push_back(cur);
        if (!check(cur)) {
            cout << "Вывод некорректен начиная с формулы номер " << line << " :\n" + error << endl;
//            cout << cur->to_string();
            return;
        }
    }
    cout << "Доказательство корректно." << endl;
}