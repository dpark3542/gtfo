#define MAXN 1000

#include <iostream>
#include <vector>
#include <sys/wait.h>
#include <fstream>
#include <regex>
#include "nauty.h"

std::string GAP = "/home/dpark/gap-4.11.1/gap";
std::vector<std::vector<int>> automorphisms;

void capture(int count, int *perm, int *orbits, int numorbits, int stabvertex, int n) {
  std::vector<int> a(n);
  for (int i = 0; i < n; i++) {
    a[i] = perm[i];
  }
  automorphisms.push_back(a);
}

// TODO: handle sets which overflow to next line
void parse(std::vector<std::vector<int>> &ans, std::string &line) {
  std::regex a(R"(\[ \[ [\d\s,]+ \] \])"), b(R"(\d+)");
  std::sregex_token_iterator it(line.begin(), line.end(), a), end;
  while (it != end) {
    std::string s(it->str());
    std::sregex_token_iterator it2(s.begin(), s.end(), b);
    std::vector<int> v;
    while (it2 != end) {
      v.push_back(std::stoi(*it2));
      it2++;
    }
    if (!v.empty()) {
      ans.push_back(v);
    }
    it++;
  }
}

// input: filename or
// n: number of vertices
// t: size of each set
// g: graph in adjacency list form
// output: all non-isomorphic subsets of t vertices
int main(int argc, char **argv) {
  std::ifstream in;
  int n, m, t;
  if (argc == 1) {
    std::cin >> n >> t;
  }
  else {
    in.open(argv[1]);
    if (!in.is_open()) {
      std::cerr << "File open failed." << std::endl;
      return 0;
    }
    in >> n >> t;
  }
  if (n > MAXN) {
    std::cerr << "Max number of vertices is " << MAXN << std::endl;
    return 0;
  }
  m = SETWORDSNEEDED(n);
  nauty_check(WORDSIZE, m, n, NAUTYVERSIONID);

  graph g[n * m];
  int lab[n], ptn[n], orbits[n];
  static DEFAULTOPTIONS_GRAPH(options);
  options.writeautoms = TRUE;
  FILE *fp = tmpfile();
  options.outfile = fp;
  options.userautomproc = capture;
  statsblk stats;

  // create graph
  EMPTYGRAPH(g, m, n);
  if (argc == 1) {
    for (int i = 0; i < n; i++) {
      std::string s;
      std::cin >> s;
      for (int j = 0; j < n; j++) {
        if (s[j] == '1') {
          ADDONEEDGE(g, i, j, m);
        }
      }
    }
  }
  else {
    for (int i = 0; i < n; i++) {
      std::string s;
      in >> s;
      for (int j = 0; j < n; j++) {
        if (s[j] == '1') {
          ADDONEEDGE(g, i, j, m);
        }
      }
    }
  }

  // call nauty
  densenauty(g, lab, ptn, orbits, &options, &stats, m, n, nullptr);
  fclose(fp);

  // set up gap child process
  int fd[2], fd2[2];
  if (pipe(fd) == -1 || pipe(fd2) == -1) {
    std::cerr << "pipe failed\n";
    exit(1);
  }
  pid_t pid = fork();
  if (pid == -1) {
    std::cerr << "fork failed";
    exit(1);
  }
  else if (pid == 0) {
    // gap child process
    close(STDIN_FILENO);
    dup(fd[0]);
    close(STDOUT_FILENO);
    dup(fd2[1]);
    close(fd[0]);
    close(fd[1]);
    close(fd2[0]);
    close(fd2[1]);
    const char *args[] = { "gap" , nullptr};
    execv(GAP.c_str(), const_cast<char *const *>(args));
    std::cerr << "execvp failed";
    exit(1);
  }
  else {
    // parent process
    int stdout_copy = dup(STDOUT_FILENO);
    close(STDIN_FILENO);
    dup(fd2[0]);
    close(STDOUT_FILENO);
    dup(fd[1]);
    close(fd[0]);
    close(fd[1]);
    close(fd2[0]);
    close(fd2[1]);

    // create automorphism group
    std::cout << "g:=Group(";
    for (const auto &a : automorphisms) {
      std::vector<bool> mkd(n);
      for (int i = 0; i < n; i++) {
        if (!mkd[i] && a[i] != i) {
          std::cout << '(';
          int j = i;
          while (a[j] != i) {
            mkd[j] = true;
            std::cout << j + 1 << ',';
            j = a[j];
          }
          mkd[j] = true;
          std::cout << j + 1 << ')';
        }
      }
      if (a != automorphisms.back()) {
        std::cout << ',';
      }
    }
    std::cout << ");;" << std::endl;

    // get orbits of t-sets
    std::cout << "l:=OrbitsDomain(g, Combinations([1.." << n << "], " << t << "), OnSets);;" << std::endl;

    // get representatives of each orbit
    std::cout << "l{[1..Length(l)]}{[1]};" << std::endl;
    std::cout << "quit;" << std::endl;
    wait(nullptr);
    dup2(stdout_copy, STDOUT_FILENO);

    // parse gap output
    std::vector<std::vector<int>> ans;
    std::string line;
    while (getline(std::cin, line)) {
      parse(ans, line);
    }
    std::cout << ans.size() << " sets" << std::endl;
    for (const auto& s : ans) {
      for (int i = 0; i < t - 1; i++) {
        std::cout << s[i] << ", ";
      }
      std::cout << s[t - 1] << std::endl;
    }
  }
}