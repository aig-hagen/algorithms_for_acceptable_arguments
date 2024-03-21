#include "clausebuffer.h"
#include "global.h"

using namespace std;
using namespace cgss2;

void ClauseBuffer::squash() {
  if (clauses.size()<2) return;
  unsigned ttl_size = 0;
  for (unsigned i=0; i<clauses.size(); ++i) {
    ttl_size += clauses[i].size();
    if (!is_dimacs[i]) {
      for (auto& cl : clauses[i]) {
        for (int& l : cl) {
          l = toDimacs(l);
        }
      }
    }
    if (clauses[i].size() > clauses[0].size()) {
      swap(clauses[i], clauses[0]);
      swap(weights[i], weights[0]);
    }
  }
  clauses[0].reserve(ttl_size);
  weights.reserve(ttl_size);
  for (unsigned i=1; i<clauses.size(); ++i) {
    clauses[0].insert(clauses[0].end(), clauses[i].begin(), clauses[i].end());
    weights[0].insert(weights[0].end(), weights[i].begin(), weights[i].end());
  }
  clauses.resize(1);
  weights.resize(1);
  is_dimacs.resize(1);
  is_dimacs[0] = 1;
}

void ClauseBuffer::_push(vector<int>& clause, uint64_t weight, bool dimacs) {
  if (!is_dimacs.size() || is_dimacs.back() != dimacs) {
    clauses.emplace_back();
    weights.emplace_back();
    is_dimacs.push_back(dimacs);
  }
  clauses.back().emplace_back();
  weights.back().push_back(weight);

  swap(clauses.back().back(), clause);
  ++nbclauses;
}

void ClauseBuffer::_push(vector<vector<int> >& _clauses, vector<uint64_t>& _weights, bool dimacs) {
  clauses.emplace_back();
  weights.emplace_back();
  is_dimacs.push_back(dimacs);
  swap(_clauses, clauses.back());
  swap(_weights, weights.back());
  nbclauses+=clauses.back().size();
}


void ClauseBuffer::clear() {
  clauses.clear();
  is_dimacs.clear();
  weights.clear();
  nbclauses = 0;
}
