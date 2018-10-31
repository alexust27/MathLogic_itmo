#include <iostream>
#include <vector>

using namespace std;

vector<vector<int> > uv;
vector<vector<int> > rev_uv;
vector<int> used;
vector<int> used2;

unsigned int n;

vector<vector<int> > sum;
vector<vector<int> > mul;
vector<vector<int> > impl;

//vector<int> split(string s, int i) {
//    vector<int> ans;
//    int x = 0;
//    for (char cc: s) {
//        if (isdigit(cc)) {
//            x = x * 10 + (cc - '0');
//        } else {
//            if (x - 1 != i)
//                ans.push_back(x - 1);
//            x = 0;
//        }
//    }
//    if (x - 1 != i)
//        ans.push_back(x - 1);
//
//    return ans;
//}

void make_rev() {
    rev_uv.resize(n);
    for (int i = 0; i < uv.size(); ++i) {
        for (int to : uv[i]) {
            rev_uv[to].push_back(i);
        }
    }
}

void dfs(int v, bool rev, bool is_v1) {
    if (is_v1)
        used[v] = true;
    else
        used2[v] = true;
    vector<int> ver;
    ver = rev ? rev_uv[v] : uv[v];
    for (int to : ver) {
        if ((is_v1 && !used[to]) || (!is_v1 && !used2[to]))
            dfs(to, rev, is_v1);
    }
}

int k2;

void dfs2(int v, bool rev) {
    if (used[v])
        k2--;
    used2[v] = true;
    vector<int> ver;
    ver = rev ? rev_uv[v] : uv[v];

    for (int to : ver) {
        if (!used2[to])
            dfs2(to, rev);
    }
}

bool check_sum_mul(bool rev) {
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            used.assign(n, 0);
            used[i] = 1;
            dfs(i, rev, true);

            used2.assign(n, 0);
            used2[j] = 1;
            dfs(j, rev, false);

            int k = 0;
            for (int ii = 0; ii < n; ++ii) {
                used[ii] = (used[ii] & used2[ii]);
                if (used[ii])
                    k++;
            }
            bool f = true;
            for (int ii = 0; ii < n; ++ii) {
                if (used[ii]) {
                    k2 = k;
                    used2.assign(n, 0);
                    dfs2(ii, rev);
                    if (k2 == 0) {
                        if (rev) {
                            mul[i][j] = mul[j][i] = ii;
                        } else {
                            sum[i][j] = sum[j][i] = ii;
                        }
                        f = false;
                        break;
                    }
                }
            }

            if (f) {
                if (!rev)
                    cout << "Операция '+' не определена: " << i + 1 << "+" << j + 1 << endl;
                else
                    cout << "Операция '*' не определена: " << i + 1 << "*" << j + 1 << endl;
                return true;
            }
        }
    }
    return false;
}

bool is_distribut() {
    for (int a = 0; a < n; ++a) {
        for (int b = 0; b < n; ++b) {
            for (int c = 0; c < n; ++c) {
                if (mul[a][sum[b][c]] != sum[mul[a][b]][mul[a][c]]) {
                    cout << "Нарушается дистрибутивность: " << a + 1 << "*(" << b + 1 << "+" << c + 1 << ")\n";
                    return true;
                }
            }
        }
    }
    return false;
}

bool is_implicative() {
    for (int a = 0; a < n; ++a) {
        for (int b = 0; b < n; ++b) {
            used.assign(n, 0);
            int k = 0;
            k2 = 0;
            for (int c = 0; c < n; ++c) {
                int ml = mul[a][c];
                used2.assign(n, 0);
                dfs2(ml, false);
                used[c] = used2[b];
                if (used[c]) k++;
            }

            bool f = true;
            for (int c = 0; c < n; ++c) {
                if (used[c]) {
                    k2 = k;
                    used2.assign(n, 0);
                    dfs2(c, true);
                    if (k2 == 0) {
                        impl[a][b] = c;
                        f = false;
                        break;
                    }
                }
            }
            if (f) {
                cout << "Операция '->' не определена: " << a + 1 << "->" << b + 1 << endl;
                return true;
            }

        }
    }
    return false;
}

void init(int i) {
    char c;
    do {
        if ((c = getchar()) != ' ' && c != '\n') {
            int x = c - '0';
            while ((c = getchar()) != ' ' && c != '\n') {
                x *= 10;
                x += c - '0';
            }
            if (i != x - 1)
                uv[i].push_back(x - 1);
        }
    } while (c != '\n');
}

int main() {
//    freopen("input.txt", "r", stdin);
//    freopen("output.txt", "w", stdout);

    scanf("%d", &n);
    getchar();
    string s;
    uv.resize(n);
//    char ch;
    for (int i = 0; i < n; ++i) {
//        s.clear();
//        ch = getchar();
//        while (ch != '\n' && ch != EOF) {
//            s.push_back(ch);
//            ch = getchar();
//        }
        init(i);
    }

    make_rev();
    uv.size();
    used.resize(n);
    used2.resize(n);

    sum.resize(n);
    for (int i = 0; i < n; ++i) {
        sum[i].resize(n);
        sum[i][i] = i;
    }

    mul.resize(n);
    for (int i = 0; i < n; ++i) {
        mul[i].resize(n);
        mul[i][i] = i;
    }

    impl.resize(n);
    for (int i = 0; i < n; ++i) {
        impl[i].resize(n);
    }

    if (check_sum_mul(false)) {
        return 0;
    }
    if (check_sum_mul(true)) {
        return 0;
    }

    if (is_distribut()) {
        return 0;
    }

    if (is_implicative())
        return 0;

    int one = impl[0][0];
    int zero = 0;
    for (int i = 0; i < n; ++i) {
        if (rev_uv[i].empty()) {
            zero = i;
            break;
        }
    }
    for (int a = 0; a < n; ++a) {
        if (sum[a][impl[a][zero]] != one) {
            cout << "Не булева алгебра: " << a + 1 << "+~" << a + 1 << endl;
            return 0;
        }
    }
    cout << "Булева алгебра\n";
    return 0;
}