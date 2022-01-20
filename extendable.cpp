#define MAXN 1000

#include <iostream>
#include <vector>
#include <sys/wait.h>
#include <fstream>
#include <regex>
#include "nauty.h"

std::string GAP = "/home/dpark/gap-4.11.1/gap";
std::vector<std::vector<int>> generators;

void capture(int count, int *perm, int *orbits, int numorbits, int stabvertex, int n) {
  std::vector<int> a(n);
  for (int i = 0; i < n; i++) {
    a[i] = perm[i];
  }
  generators.push_back(a);
}

unsigned int next(unsigned int x) {
  int a = 0, b = 0;
  while ((x & 3) != 1) {
    if ((x & 1) == 1) {
      a++;
    }
    b++;
    x >>= 1;
  }
  return ((x + 1) << b) + (1 << a) - 1;
}

unsigned int prev(unsigned int x) {
  int a = 0, b = 0;
  while ((x & 3) != 2) {
    if ((x & 1) == 0) {
      a++;
    }
    b++;
    x >>= 1;
  }
  return ((x ^ 3) << b) + (1 << b) - (1 << a);
}

//void print_set(unsigned int x) {
//  for (int i = 1; x != 0; i++,x>>=1) {
//    if (x & 1) {
//      std::cout << i << ' ';
//    }
//  }
//  std::cout << std::endl;
//}

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
    for (const auto &gen : generators) {
      std::vector<bool> mkd(n);
      for (int i = 0; i < n; i++) {
        if (!mkd[i] && gen[i] != i) {
          std::cout << '(';
          int j = i;
          while (gen[j] != i) {
            mkd[j] = true;
            std::cout << j + 1 << ',';
            j = gen[j];
          }
          mkd[j] = true;
          std::cout << j + 1 << ')';
        }
      }
      if (gen != generators.back()) {
        std::cout << ',';
      }
    }
    std::cout << ");;" << std::endl;

    std::cout << "SetPrintFormattingStatus(\"*stdout*\", false);;" << std::endl;
//    std::cout << "AsList(g);" << std::endl;
    std::cout << "Orbit(g, [1.." << n << "], OnTuples);" << std::endl;

    std::string line;
    while (!line.starts_with("gap> gap> gap> [ ")) {
      getline(std::cin, line);
    }

    std::cout << "quit;" << std::endl;
    wait(nullptr);
    dup2(stdout_copy, STDOUT_FILENO);
    std::vector<std::vector<int>> automorphisms;
    std::regex r(R"(\d+)");
    std::sregex_token_iterator it(line.begin(), line.end(), r), end;
    while (it != end) {
      std::vector<int> a(n);
      for (int i = 0; i < n; i++) {
        a[i] = std::stoi(*it) - 1;
        it++;
      }
      automorphisms.push_back(a);
    }

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<int>> ans;
    std::map<unsigned int, unsigned int> map{{(1 << t) - 1, ((1 << t) - 1) << (n - t)}};
    while (!map.empty()) {
      unsigned int min = map.begin()->first;

      std::vector<int> s;
      for (int i = 1, x = min; x != 0; i++, x>>=1) {
        if (x & 1) {
          s.push_back(i);
        }
      }
      ans.push_back(s);

      for (const auto &a : automorphisms) {
        unsigned int im = 0;
        for (int i = n - 1; i >= 0; i--) {
          im = (im << 1) + ((min >> a[i]) & 1);
        }
        auto l = map.lower_bound(im);
        if (l != map.end() && im == l->first) {
          if (im == l->second) {
            map.erase(l);
          }
          else {
            auto entry = map.extract(l->first);
            entry.key() = next(im);
            map.insert(std::move(entry));
          }
        }
        else if (l != map.begin()) {
          l--;
          if (im == l->second) {
            l->second = prev(l->second);
          }
          else if (im < l->second) {
            map[next(im)] = l->second;
            l->second = prev(im);
          }
        }
      }
    }

    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> d = stop - start;
    std::cout << d.count() << "ms" << std::endl;

//    // print answer
//    std::cout << ans.size() << " sets" << std::endl;
//    for (const auto& s : ans) {
//      for (int i = 0; i < t - 1; i++) {
//        std::cout << s[i] << ", ";
//      }
//      std::cout << s[t - 1] << std::endl;
//    }
  }
}