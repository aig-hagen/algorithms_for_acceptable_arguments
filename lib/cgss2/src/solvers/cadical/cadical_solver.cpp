#include "cadical_solver.h"
#include "../../global.h"
#include "../../../cadical/src/version.hpp"

#include <cstring>
#include <iostream>
#include <sstream>

using namespace std;
using namespace cgss2;

void CadicalSolver::add_clause(const vector<int>& clause) {
  for (int l : clause) {
    ccadical_add(solver, toDimacs(l));
    max_var = max(max_var, abs(toDimacs(l)));
  }
  ccadical_add(solver, 0);
}

void CadicalSolver::add_clause(int lit) {
  ccadical_add(solver, toDimacs(lit));
  max_var = max(max_var, abs(toDimacs(lit)));
  ccadical_add(solver, 0);
}
void CadicalSolver::add_clause(int lit1, int lit2) {
  ccadical_add(solver, toDimacs(lit1));
  ccadical_add(solver, toDimacs(lit2));
  max_var = max(max_var, abs(toDimacs(lit1)));
  max_var = max(max_var, abs(toDimacs(lit2)));
  ccadical_add(solver, 0);
}

void CadicalSolver::add_clause(int lit1, int lit2, int lit3) {
  ccadical_add(solver, toDimacs(lit1));
  ccadical_add(solver, toDimacs(lit2));
  ccadical_add(solver, toDimacs(lit3));
  max_var = max(max_var, abs(toDimacs(lit1)));
  max_var = max(max_var, abs(toDimacs(lit2)));
  max_var = max(max_var, abs(toDimacs(lit3)));
  ccadical_add(solver, 0);
}

bool CadicalSolver::solve(vector<int>& assumptions) {
  prev_assumps.resize(assumptions.size());
  memcpy(prev_assumps.data(), assumptions.data(), assumptions.size()*sizeof(int));
  for (int l : assumptions) {
    ccadical_assume(solver, toDimacs(l));
    max_var = max(max_var, abs(toDimacs(l)));
  }
  int result = ccadical_solve(solver);
  return result==10;
}

bool CadicalSolver::solve(int lit) {
  prev_assumps.resize(1);
  prev_assumps[0]=lit;
  ccadical_assume(solver, toDimacs(lit));
  max_var = max(max_var, abs(toDimacs(lit)));
  int result = ccadical_solve(solver);
  return result==10;
}

bool CadicalSolver::solve() {
  int result = ccadical_solve(solver);
  return result==10;
}



void CadicalSolver::set_budget(int64_t) { }

void CadicalSolver::set_budget_relative(double) { }

void CadicalSolver::increase_budget(int64_t) { }

void CadicalSolver::increase_budget_relative(double) { }

int CadicalSolver::solve_limited(std::vector<int>&) {
  return -1;
}

int CadicalSolver::core_size() {
  int sz = 0;
  for (int& i : prev_assumps) {
    if (ccadical_failed(solver, toDimacs(i))) {
      ++sz;
    }
  }
  return sz;
}

void CadicalSolver::get_core(vector<int>& retCore) {
  retCore.clear();
  for (int& i : prev_assumps) {
    if (ccadical_failed(solver, toDimacs(i))) {
      retCore.push_back(i);
    }
  }
}

void CadicalSolver::get_model(vector<bool>& retModel) {
  retModel.clear();
  for (int var = 1; var <= max_var; ++var) {
    if (ccadical_val(solver, var)==var) {
      retModel.push_back(1);
    } else {
      retModel.push_back(0);
    }
  }
}


bool CadicalSolver::propagate(vector<int>&, vector<int>&){
  return 1;
}

bool CadicalSolver::propagate(int, vector<int>&) {
  return 1;
}

bool CadicalSolver::prop_check(vector<int>&) {
  return 1;
}

bool CadicalSolver::prop_check(int) {
  return 1;
}

void CadicalSolver::get_learnt_clauses(std::vector<std::vector<int> >&) {

}

void CadicalSolver::stats(const std::string&, std::ostream&) {
  // TODO
}

void CadicalSolver::set_option(std::string&) {
  // TODO
}

std::string CadicalSolver::version(int l) {
  std::stringstream vs;
  if (l&1) vs << "CaDiCaL ";
  vs << CaDiCaL::version();
  if (l&2) vs << " (" << CaDiCaL::identifier() << ", " << CaDiCaL::date() << ")";
  return vs.str();
}


CadicalSolver::CadicalSolver() {
  solver = ccadical_init();
  ccadical_set_option(solver, "stabilize", 0);
  ccadical_set_option(solver, "walk", 0);
  max_var = 1;
}
