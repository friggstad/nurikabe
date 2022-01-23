/*
  Nurikabe solver.
  Zachary Friggstad, 2020
*/

#include <bits/stdc++.h>

#define EMPTY 0
#define BLACK -1
#define DOT -2
#define UF_PAD 8

using namespace std;

using vi = vector<int>;
using vvi = vector<vi>;
using pii = pair<int, int>;

struct entry {
  int r, c, v;
};

bool step;

int R, C; // rows, columns
vector<entry> givens;
vvi grid;
vector<vector<vector<pii>>> order;

inline int pack(int i, int j) {
  return (i<<UF_PAD)|j;
}

// union-find data structure for grid squares (i,j)
// also tracks the size of each component
// uses path compression only
struct UnionFind {
  UnionFind(int R = 1, int C = 1) {
    assert(C < (1<<UF_PAD));

    this->R = R;
    this->C = C;
    uf.resize(R*(1<<UF_PAD));
    sz = uf;
    reset();
  }

  void reset() {
    for (int i = 0; i < R; ++i)
      for (int j = 0; j < C; ++j) {
        int p = pack(i,j);
        uf[p] = p;
        sz[p] = 1;
      }
  }

  int find(int x) {
    return x == uf[x] ? x : (uf[x] = find(uf[x]));
  }

  int find(int i, int j) {
    return find(pack(i, j));
  }

  bool merge(int x, int y) {
    x = find(x);
    y = find(y);
    if (x == y) return false;
    sz[y] += sz[x];
    uf[x] = y;
    return true;
  }

  bool merge(int i, int j, int ii, int jj) {
    return merge(pack(i, j), pack(ii, jj));
  }

  int cnt(int x) {
    return sz[find(x)];
  }

  int cnt(int i, int j) {
    return cnt(pack(i, j));
  }

  int R, C;
  vector<int> uf, sz;
};

UnionFind uf;

// print the board, highlight cell (ii, jj) if it lies on the board
void dump(const vvi& g, int ii = -2, int jj = -2) {
  // for values > 10, use a special chacter
  string special = "ABCDEFGHIJKLMNPQRSTUVWXYZ";

  vector<int> vals;

  for (int i = 0; i < R; ++i) {
    for (int j = 0; j < C; ++j) {
      if (i == -1) cout << (j == jj ? '@' : ' ');
      else if (j == -1) cout << (i == ii ? '@' : ' ');
      else {
        int x = g[i][j];
        if (i == ii && j == jj) cout << "\033[1;103m";

        if (x == BLACK) cout << '#';
        else if (x == DOT) cout << '.';
        else if (x == EMPTY) cout << '_';
        else if (x >= 10) {

          // whoops, too many values > 10 on the board!
          if (vals.size() >= special.size()) {
            cerr << "Too many values > 10 in the input grid, cannot display" << endl;
            assert(vals.size() < special.size());
          }

          cout << special[vals.size()];
          vals.push_back(x);
        }
        else cout << x;

        if (i == ii && j == jj) cout << "\033[0m";
      }
    }
    cout << endl;
  }
  for (int i = 0; i < vals.size(); ++i) {
    cout << special[i] << " = " << vals[i] << endl;
  }

  cout << endl;
}


bool valid(int i, int j) {
  return 0 <= i && i < R && 0 <= j && j < C;
}

int dr[] = {-1, 1, 0, 0}, dc[] = {0, 0, -1, 1};

// Helper function to see how large the potential region is from a given
// numbered cell.
// rep is the initial numbered cell (packed to a single int)
// adj[q] is the set of grid values that have q adjacent to their current region
int dfs_expand(int r, int c, int rep, const vvi& g, map<int, set<int>>& adj, set<int>& seen) {
  if (!valid(r, c)) return 0;
  int p = pack(r, c);
  if (seen.find(p) != seen.end()) return 0;
  seen.insert(p);

  if (g[r][c] == BLACK) return 0;
  if (adj[p].size() >= 2) return 0; // could not put . here, would merge 2 regions
  if (adj[p].size() == 1 && *adj[p].begin() != rep) return 0; // adjacent to a different region already

  int tot = 1;
  for (int k = 0; k < 4; ++k) tot += dfs_expand(r+dr[k], c+dc[k], rep, g, adj, seen);

  return tot;
}

// check if the grid is consistent so far
bool consistent(const vvi& g) {
  // CHECK: 2x2 squares
  for (int i = 1; i < R; ++i)
    for (int j = 1; j < C; ++j)
      if (g[i][j] == BLACK && g[i-1][j] == BLACK && g[i][j-1] == BLACK && g[i-1][j-1] == BLACK)
        return false;

  // CHECK: black regions are connected or have the potential to connect
  // (i.e. when running connectivity using black or empty, all blacks are
  // in the same component)
  uf.reset();
  for (int i = 0; i < R; ++i)
    for (int j = 0; j < C; ++j)
      if (g[i][j] == EMPTY || g[i][j] == BLACK) {
        if (i > 0 && (g[i-1][j] == EMPTY || g[i-1][j] == BLACK)) uf.merge(i, j, i-1, j);
        if (j > 0 && (g[i][j-1] == EMPTY || g[i][j-1] == BLACK)) uf.merge(i, j, i, j-1);
      }

  bool found_black = false;
  int black_rep;
  for (int i = 0; i < R; ++i)
    for (int j = 0; j < C; ++j)
      if (g[i][j] == BLACK) {
        if (!found_black) {
          black_rep = uf.find(i, j);
          found_black = true;
        }
        else if (black_rep != uf.find(i, j)) {
          return false;
        }
      }


  // CHECK: dotted regions have ok size (not too big yet)
  // and each has <= 1 given number
  uf.reset();
  for (int i = 0; i < R; ++i)
    for (int j = 0; j < C; ++j)
      if (g[i][j] == DOT || g[i][j] >= 1) {
        if (i > 0 && (g[i-1][j] == DOT || g[i-1][j] >= 1)) uf.merge(i, j, i-1, j);
        if (j > 0 && (g[i][j-1] == DOT || g[i][j-1] >= 1)) uf.merge(i, j, i, j-1);
      }

  set<int> numbered_regions;
  for (int i = 0; i < R; ++i)
    for (int j = 0; j < C; ++j)
      if (g[i][j] >= 1) {
        int rep = uf.find(i, j);
        if (numbered_regions.find(rep) != numbered_regions.end()) return false;
        numbered_regions.insert(rep);
        if (uf.cnt(rep) > g[i][j]) return false;
      }

  // CHECK: numbered regions are not trapped (i.e. can expand large enough)

  map<int, set<int>> adj;
  set<int> adj_blank;

  for (int i = 0; i < R; ++i)
    for (int j = 0; j < C; ++j) {
      int p = pack(i, j);
      int rep = uf.find(p);
      if (numbered_regions.find(rep) != numbered_regions.end()) {
        adj[p].insert(rep);
      }
      else if (g[i][j] == EMPTY) {
        for (int k = 0; k < 4; ++k) {
          int ni = i + dr[k], nj = j + dc[k];
          if (!valid(ni, nj)) continue;
          int nrep = uf.find(ni, nj);
          if (numbered_regions.find(nrep) != numbered_regions.end()) adj[p].insert(nrep);
          adj_blank.insert(nrep); // indicate the region nrep is adjacent to a blank region
        }
      }
    }

  // CHECK: No dotted regions without numbers are entirely closed off

  // TODO (OPTIMIZATION): REPLACE WITH BFS THAT TERMINATES EARLY IF LARGE ENOUGH
  for (int i = 0; i < R; ++i)
    for (int j = 0; j < C; ++j)
      if (g[i][j] >= 1) {
        set<int> seen; // can optimize further by making single array and using current iteration to hash
        int visited = dfs_expand(i, j, uf.find(i, j), g, adj, seen);
        // cout << i << ' ' << j << ' ' << g[i][j] << ' ' << visited << endl << endl;
        if (visited < g[i][j]) return false;
      }
      else if (g[i][j] == DOT) {
        int rep = uf.find(i, j);
        if (numbered_regions.find(rep) == numbered_regions.end() && adj_blank.find(rep) == adj_blank.end()) return false;
      }


  // TODO - PRIORITY:
  // Ensure all dotted regions without numbers can reach a numbered
  // region with appropriate size

  return true;
}


// returns 0 if a depth "max_depth" search discovers it is not solvable
// 1 otherwise, will return as much as could be filled out with the given depth in this case
// pi,pj is the "centre" of the search, meaning we scan grid cells nearest to (pi,pj) first
int iterate(vvi& g, int pi, int pj, int max_depth, bool report = true) {
  if (max_depth == 0) return consistent(g);

  vvi orig = g;

  for (int depth = 1; depth <= max_depth; ++depth) {
    bool progress = false;
    for (const pii& cell : order[pi][pj]) {
      int i = cell.first, j = cell.second;
      if (g[i][j] != EMPTY) continue;

      // if (report && depth >= 2) cout << "Trying " << i << ' ' << j << " at depth " << depth << endl;

      // try putting a . at i,j first
      vvi tmp = g;
      tmp[i][j] = DOT;
      int dot_ret = iterate(tmp, i, j, depth-1, false);

      if (dot_ret == 0) {
        // if . lead to contradiction, set it to # and note we made progress
        g[i][j] = BLACK;
        progress = true;
      }
      else {
        // if . did not lead to contradiction, try #
        tmp = g;
        tmp[i][j] = BLACK;
        int black_ret = iterate(tmp, i, j, depth-1, false);

        // if this led to a contradiction, fix . there and note progress
        if (black_ret == 0) {
          g[i][j] = DOT;
          progress = true;
        }
      }

      // if we made progress by fixing a mark
      if (progress) {

        // if that fixing yields an immediate contradiction, the input
        // board state is not solvable!
        if (!consistent(g)) {
          g = orig;
          return 0;
        }

        if (report) {
          cout << "Change: " << i << ' ' << j << endl << "Depth: " << depth << endl;
          dump(g, i, j);
          if (step) cin.get();
        }

        // this was the new change
        pi = i;
        pj = j;
        break;
      }
    }

    // if we fixed a mark, then resume our search but from depth 1 again
    // (outer for loop will increase the depth to 1)
    if (progress) depth = 0;
  }

  return 1;
}

// check if the board g is fully solved
bool solved(const vvi& g) {
  if (!consistent(g)) return false;
  for (int i = 0; i < R; ++i)
    for (int j = 0; j < C; ++j)
      if (g[i][j] == EMPTY)
        return false;
  return true;
}


int main(int argc, char *argv[]) {
  if (argc != 2 && argc != 3) {
    cerr << "Usage: ./nurikabe puzzlefile [--step]" << endl;
    exit(1);
  }

  ifstream fin(argv[1]);
  if (!fin) {
    cout << "Bad file" << endl;
    exit(1);
  }

  if (argc == 3) {
    assert(string(argv[2]) == "--step");
    step = true;
  }

  assert(fin >> R >> C);

  uf = UnionFind(R, C);

  grid = vvi(R, vi(C, 0));
  int rr, cc, tot = 0, num = 0;
  while (fin >> rr >> cc) {
    assert(fin >> grid[rr][cc]);
    tot += grid[rr][cc];
    ++num;
  }
  // num == # of given numbers, tot = total of these numbers

  cout << "Solving:" << endl;
  dump(grid);
  if (step) cin.get();

  vvi curr = grid;

  order.resize(R, vector<vector<pii>>(C));

  // precompute for every grid square (i,j)
  // an ordering of the other grid squares by Manhattan distance
  // Idea: when we successfully place a mark then it is likely we
  // can place another mark nearby so start searching there.
  for (int i = 0; i < R; ++i)
    for (int j = 0; j < C; ++j) {
      vector<pair<int, pii>> ord;
      for (int ii = 0; ii < R; ++ii)
        for (int jj = 0; jj < C; ++jj)
          ord.push_back({abs(i-ii) + abs(j-jj), {ii, jj}});
      sort(ord.begin(), ord.end());
      for (const auto& p : ord) order[i][j].push_back(p.second);
    }

  // let's solve this!
  int ret = iterate(curr, 0, 0, R*C+1);

  if (!ret) cout << "No Solution!" << endl;
  else {
    assert(ret == 1 && solved(curr));
    dump(curr);
  }


  return 0;
}
