#include <vector>
#include <iostream>
#include <unordered_set>
#include <unistd.h>
#include <sys/wait.h>
#include <ext/pb_ds/assoc_container.hpp>

int LIM = 10;
std::string CWD = "/home/dpark/hamming/", NAUTY = "/home/dpark/nauty27r1/";

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

__gnu_pbds::gp_hash_table<unsigned int, int> neighborhood(const graph& g) {
  int n = g.v.size();
  std::vector<bool> mkd(64);
  std::vector<unsigned int> tag(64);
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
        if (std::popcount(tag[w]) == 3) {
          mkd[w] = true;
        }
        else {
          tag[w] = tag[w] | (1 << i);
        }
      }
    }
  }
  __gnu_pbds::gp_hash_table<unsigned int, int> map;
  for (int v = 0; v < 64; v++) {
    if (!mkd[v] && tag[v] != 0) {
      map[tag[v]] = v;
    }
  }
  return map;
}

// print single graph in amtog input style
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
  std::cout << std::endl;
}

// pass vector of graphs through amtog to file
void print_graphs(const std::vector<graph> &a) {
  int fd[2];
  if (pipe(fd) == -1) {
    std::cerr << "pipe failed\n";
    exit(1);
  }
  pid_t pid = fork();
  if (pid == -1) {
    std::cerr << "fork failed";
    exit(1);
  }
  else if (pid == 0) {
    close(STDIN_FILENO);
    dup(fd[0]);
    close(fd[1]);
    close(fd[0]);
    std::string outfile = CWD + std::to_string(a[0].v.size()) + ".g6", amtog = NAUTY + "amtog";
    const char *argv[] = {"amtog", "-", outfile.c_str(), nullptr};
    execvp(amtog.c_str(), const_cast<char *const *>(argv));
    std::cerr << "execvp failed";
    exit(1);
  }
  else {
    int stdout_copy = dup(STDOUT_FILENO);
    close(STDOUT_FILENO);
    dup(fd[1]);
    close(fd[0]);
    close(fd[1]);
    for (const graph &g : a) {
      print_graph(g);
    }
    std::cout << "q" << std::endl;
    wait(nullptr);
    dup2(stdout_copy, STDOUT_FILENO);
  }
}

// prune graphs in file with shortg
std::vector<int> unique_graphs(int n) {
  int fd[2];
  if (pipe(fd) == -1) {
    std::cerr << "pipe failed\n";
    exit(1);
  }
  pid_t pid = fork();
  if (pid == -1) {
    std::cerr << "fork failed";
    exit(1);
  }
  else if (pid == 0) {
    close(STDERR_FILENO);
    dup(fd[1]);
    close(fd[0]);
    close(fd[1]);
    std::string infile = CWD + std::to_string(n) + ".g6", shortg = NAUTY + "shortg";
    const char *argv[] = {"shortg", infile.c_str(), "-v", nullptr};
    execvp(shortg.c_str(), const_cast<char *const *>(argv));
    std::cout << "execvp failed";
    exit(1);
  }
  else {
    close(STDIN_FILENO);
    dup(fd[0]);
    close(fd[1]);
    close(fd[0]);
    int cnt = 0;
    std::string s;
    std::vector<int> a;
    while (cnt < 2) {
      std::cin >> s;
      if (s == ">Z") {
        cnt++;
      }
      else if (s == ":") {
        int x;
        std::cin >> x;
        a.push_back(x);
      }
    }
    wait(nullptr);
    return a;
  }
}

int main() {
  init();

  // initialize n = 4
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

  for (int n = 4; n < LIM; n++) {
    std::cout << "enumerating" << std::endl;
    std::vector<graph> b;
    for (const graph &g : a) {
      __gnu_pbds::gp_hash_table<unsigned int, int> add = neighborhood(g);
      for (auto [k, v] : add) {
        graph h = g;
        h.v.push_back(v);
        for (unsigned int w = k, i = 0; w != 0; w >>= 1, i++) {
          if (w & 1) {
            h.e[v].push_back(h.v[i]);
            h.e[h.v[i]].push_back(v);
          }
        }
        b.push_back(h);
      }
    }
    a = std::move(b);
    b.clear();
    std::cout << a.size() << " graphs" << std::endl;
    std::cout << "writing" << std::endl;
    print_graphs(a);
    std::cout << "unique" << std::endl;
    std::vector<int> ind = unique_graphs(n + 1);
    for (int i : ind) {
      b.push_back(a[i - 1]);
    }
    a = std::move(b);
    std::cout << n + 1 << ": " << a.size() << " graphs" << std::endl;
  }
  return 0;
}
