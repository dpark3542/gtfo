#include <vector>
#include <iostream>
#include <unordered_set>

std::vector<int> expand(int x) {
  return {(x / 16) % 4, (x / 4) % 4, x % 4};
}

struct graph {
  std::vector<int> v;
  std::vector<std::vector<int>> e; // always size 64
} ham;

// generate H(3,4)
void init() {
  for (int i = 0; i < 64; i++) {
    ham.v.push_back(i);
    ham.e.resize(64);
  }
  for (int i = 0; i < 64; i++) {
    std::vector<int> u = expand(i);
    for (int j = i + 1; j < 64; j++) {
      std::vector<int> v = expand(j);
      int dist = 0;
      for (int k = 0; k < 3; k++) {
        if (u[k] != v[k]) {
          dist++;
        }
      }
      if (dist == 1) {
        ham.e[i].push_back(j);
        ham.e[j].push_back(i);
      }
    }
  }
}

std::vector<std::pair<int, int>> f(const graph& g) {
  int n = g.v.size();
  std::vector<bool> mkd(64);
  std::vector<int> map(64);
  for (int v : g.v) {
    mkd[v] = true;
    if (g.e[v].size() == 3) {
      for (int w : ham.e[v]) {
        mkd[w] = true;
      }
    }
  }
  for (int i = 0; i < n; i++) {
    for (int w : ham.e[g.v[i]]) {
      if (!mkd[w]) {
        map[w] = map[w] | (1 << i);
      }
    }
  }
  std::unordered_set<int> s;
  std::vector<std::pair<int, int>> ans;
  for (int v = 0; v < 64; v++) {
    if (map[v] != 0 && !s.contains(map[v])) {
      ans.emplace_back(v, map[v]);
      s.insert(map[v]);
    }
  }
  return ans;
}

// print graph in amtog input style
void print_graph(const graph &g) {
  int n = g.v.size();
  std::cout << "n=" << n << '\n';
  std::vector<int> ind(64);
  for (int i = 0; i < n; i++) {
    ind[g.v[i]] = i;
  }
  for (int i = 0; i < n; i++) {
    std::vector<char> a(n, '0');
    for (int w : g.e[g.v[i]]) {
      a[ind[w]] = '1';
    }
    for (char c : a) {
      std::cout << c;
    }
    std::cout << '\n';
  }
  std::cout << '\n';
}

int main() {
  int lim = 5;
//  if (argc == 2) {
//    lim = atoi(argv[1]);
//  }

  init();

  // initialize f(4)
  std::vector<graph> a(2);
  a[0].v = {0, 1, 4, 16};
  a[0].e.resize(64);
  a[0].e[0] = {1, 4, 16};
  a[0].e[1] = {0};
  a[0].e[4] = {0};
  a[0].e[16] = {0};
  a[1].v = {0, 1, 2, 4};
  a[1].e.resize(64);
  a[1].e[0] = {1, 2, 4};
  a[1].e[1] = {0, 2};
  a[1].e[2] = {0, 1};
  a[1].e[4] = {0};

  // next step
  for (int n = 4; n < lim; n++) {
    std::vector<graph> b;
    for (const graph &g : a) {
      std::vector<std::pair<int, int>> vl = f(g);
      for (auto[v, w] : vl) {
        graph h = g;
        h.v.push_back(v);
        for (int i = 0; w != 0; i++, w >>= 1) {
          if (w & 1) {
            h.e[v].push_back(h.v[i]);
            h.e[h.v[i]].push_back(v);
          }
        }
        b.push_back(h);
      }
    }
    a = std::move(b);
    for (const graph &g : a) {
      print_graph(g);
    }
    std::cout << "q\n";
  }
  return 0;
}
