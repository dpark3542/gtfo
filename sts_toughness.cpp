#include <iostream>
#include <fstream>
#include <vector>
#include <deque>

/*
 * input file sts13.txt created by running:
 * In[1]:= Thread@Rule[VertexList@STSGraph[13],
  Range[0, VertexCount@STSGraph[13] - 1]]
 * In[2]:= StringJoin[
 ToString[#1] <> " " <> ToString[#2] <>
    "\n" & @@@ (EdgeList@STSGraph[13] /. %)]
 */

int count_components(int n, std::vector<std::vector<int>> &g, uint32_t s) {
  int c = 0;
  std::vector<bool> mkd(n), h(n);
  for (int i = 1; i < n; i++) {
    h[i] = !(s & (1u << (unsigned) (i - 1)));
//    if (!h[i]) {
//      std::cout << i << ' ';
//    }
  }
//  std::cout << std::endl;
  for (int u = 0; u < n; u++) {
    if (h[u] && !mkd[u]) {
      c++;
      std::deque<int> st;
      st.push_back(u);
      while (!st.empty()) {
        int v = st.back();
        st.pop_back();
        if (!mkd[v]) {
          mkd[v] = true;
          for (int w : g[v]) {
            if (h[w]) {
              st.push_back(w);
            }
          }
        }
      }
    }
  }
  return c;
}

int main() {
  std::ifstream in;
  in.open("sts13.txt");
  if (!in.is_open()) {
    std::cerr << "File open failed." << std::endl;
    return 0;
  }
  int n = 26, m = 195;
  std::vector<std::vector<int>> g(n);
  for (int i = 0; i < m; i++) {
    int u, v;
    in >> u >> v;
    g[u].push_back(v);
    g[v].push_back(u);
  }

//  std::cout << count_components(n, g, 0) << std::endl;
//  std::cout << count_components(n, g, 0b11111111111101111010111011) << std::endl;
//  std::cout << count_components(n, g, 0b11111111111111111111111111) << std::endl;

  int tn = 1000000, td = 1;
  uint32_t max_s, ub = 1u << (unsigned) (n - 1);
  for (uint32_t s = 0; s < ub - 1; s++) {
    if (s % 100000 == 0) {
      std::cout << s << ": " << tn << ' ' << td << std::endl;
    }
    int c = 0, a = __builtin_popcount(s) + 1;
    c = count_components(n, g, s);
    if (c >= 2 && a * td < tn * c) {
      tn = a;
      td = c;
      max_s = s;
    }
  }
  std::cout << tn << ' ' << td << ' ' << max_s << std::endl;
  return 0;
}
