#include "glucose3x_solver.h"
#include "../../global.h"

#include <cstring>
#include <iostream>

using namespace std;

void Glucose3xSolver::add_clause(const vector<int>& clause) {
  if (clause.size()==1) {
    while ((clause[0]>>1)>topv) {solver.newVar();++topv;}
    solver.addClause(Glucose3xCGSS2::toLit(clause[0]));
  } else if (clause.size() == 2) {
    while ((clause[0]>>1)>topv || (clause[1]>>1)>topv) {solver.newVar();++topv;}
    solver.addClause(Glucose3xCGSS2::toLit(clause[0]), Glucose3xCGSS2::toLit(clause[1]));
  } else if (clause.size() == 3) {
    while ((clause[0]>>1)>topv || (clause[1]>>1)>topv || (clause[2]>>1)>topv) {solver.newVar();++topv;}
    solver.addClause(Glucose3xCGSS2::toLit(clause[0]), Glucose3xCGSS2::toLit(clause[1]), Glucose3xCGSS2::toLit(clause[2]));
  } else {
    for (unsigned i=0;i<clause.size();++i) while ((clause[i]>>1)>topv) {solver.newVar();++topv;}
    Glucose3xCGSS2::vec<Glucose3xCGSS2::Lit> clause_;
    intsToLits(clause, clause_);
    solver.addClause_(clause_);
  }
}

void Glucose3xSolver::add_clause(int lit) {
  while ((lit>>1)>topv) {solver.newVar();++topv;}
  solver.addClause(Glucose3xCGSS2::toLit(lit));
}
void Glucose3xSolver::add_clause(int lit1, int lit2) {
  while ((lit1>>1)>topv || (lit2>>1)>topv) {solver.newVar();++topv;}
  solver.addClause(Glucose3xCGSS2::toLit(lit1), Glucose3xCGSS2::toLit(lit2));
}

void Glucose3xSolver::add_clause(int lit1, int lit2, int lit3) {
  while ((lit1>>1)>topv || (lit2>>1)>topv  || (lit3>>1)>topv) {solver.newVar();++topv;}
  solver.addClause(Glucose3xCGSS2::toLit(lit1), Glucose3xCGSS2::toLit(lit2), Glucose3xCGSS2::toLit(lit3));
}

bool Glucose3xSolver::solve(vector<int>& assumptions) {
  Glucose3xCGSS2::vec<Glucose3xCGSS2::Lit> assumps;
  intsToLits(assumptions, assumps);
  bool rv = solver.solve(assumps);
  return rv;
}

bool Glucose3xSolver::solve(int lit) {
  while ((lit>>1)>topv) {solver.newVar();++topv;}
  bool rv = solver.solve(Glucose3xCGSS2::toLit(lit));
  return rv;
}


bool Glucose3xSolver::solve() {
  bool rv = solver.solve();
  return rv;
}

void Glucose3xSolver::set_budget(int64_t max_conflicts) {
  solver.setConfBudget(max_conflicts);
}

void Glucose3xSolver::set_budget_relative(double budget) {
  uint64_t max_conflicts = max((uint64_t)1, (uint64_t)(solver.conflicts*budget));
  set_budget(max_conflicts);
}

void Glucose3xSolver::increase_budget(int64_t max_conflicts) {
  solver.increaseConfBudget(max_conflicts);
}

void Glucose3xSolver::increase_budget_relative(double budget) {
  uint64_t max_conflicts = max((uint64_t)1, (uint64_t)(solver.conflicts*budget));
  increase_budget(max_conflicts);
}


int Glucose3xSolver::solve_limited(vector<int>& assumptions) {
  Glucose3xCGSS2::vec<Glucose3xCGSS2::Lit> assumps;
  intsToLits(assumptions, assumps);

  Glucose3xCGSS2::lbool srv = solver.solveLimited(assumps);

  int rv;
  if (srv == g3xl_TrueP)     rv=1;
  else if (srv==g3xl_FalseP) rv=0;
  else if (srv==g3xl_UndefP) rv=-1;
  else                    rv=-2;
  return rv;
}


int Glucose3xSolver::core_size() {
  return solver.conflict.size();
}

void Glucose3xSolver::get_core(vector<int>& retCore) {
	litsToInts(solver.conflict, retCore, true);
}

void Glucose3xSolver::get_model(vector<bool>& retModel) {
  lboolsToBools(solver.model, retModel);
}


bool Glucose3xSolver::propagate(vector<int>& assumptions, vector<int>& result){
  Glucose3xCGSS2::vec<Glucose3xCGSS2::Lit> assumps;
  intsToLits(assumptions, assumps);
  Glucose3xCGSS2::vec<Glucose3xCGSS2::Lit> res;
  bool rv = solver.prop_check(assumps, res, 0);
  litsToInts(res, result);
  return rv;
}

bool Glucose3xSolver::propagate(int lit, vector<int>& result) {
  while ((lit>>1)>topv) {solver.newVar();++topv;}
  Glucose3xCGSS2::vec<Glucose3xCGSS2::Lit> assumps;
  assumps.push(Glucose3xCGSS2::toLit(lit));
  Glucose3xCGSS2::vec<Glucose3xCGSS2::Lit> res;
  bool rv = solver.prop_check(assumps, res, 0);
  litsToInts(res, result);
  return rv;
}

bool Glucose3xSolver::prop_check(vector<int>& assumptions) {
  Glucose3xCGSS2::vec<Glucose3xCGSS2::Lit> assumps;
  intsToLits(assumptions, assumps);
  bool rv = solver.prop_confl_check(assumps, 0);
  return rv;
}

bool Glucose3xSolver::prop_check(int lit) {
  while ((lit>>1)>topv) {solver.newVar();++topv;}
  Glucose3xCGSS2::vec<Glucose3xCGSS2::Lit> assumps;
  assumps.push(Glucose3xCGSS2::toLit(lit));
  bool rv = solver.prop_confl_check(assumps, 0);
  return rv;
}


void Glucose3xSolver::get_learnt_clauses(vector<vector<int> >& clauses) {
  const Glucose3xCGSS2::vec<Glucose3xCGSS2::CRef>& learnts = solver.get_learnts();
  for (int i = 0; i<learnts.size(); ++i) {
    const Glucose3xCGSS2::Clause& c = solver.get_clause(learnts[i]);
    clauses.emplace_back(c.size());
    for (int j = 0; j<c.size(); ++j) {
      clauses.back()[j]=Glucose3xCGSS2::toInt(c[j]);
    }
  }
}

void Glucose3xSolver::stats(const std::string& b, std::ostream& out) {
  out << b << "nbRemovedClauses: "   << solver.nbRemovedClauses << "\n";
  out << b << "nbReducedClauses: "   << solver.nbReducedClauses << "\n";
  out << b << "nbDL2: "              << solver.nbDL2 << "\n";
  out << b << "nbBin: "              << solver.nbBin << "\n";
  out << b << "nbUn: "               << solver.nbUn << "\n";
  out << b << "nbReduceDB: "         << solver.nbReduceDB << "\n";
  out << b << "solves: "             << solver.solves << "\n";
  out << b << "starts: "             << solver.starts << "\n";
  out << b << "decisions: "          << solver.decisions << "\n";
  out << b << "rnd_decisions: "      << solver.rnd_decisions << "\n";
  out << b << "propagations: "       << solver.propagations << "\n";
  out << b << "conflicts: "          << solver.conflicts << "\n";
  out << b << "conflictsRestarts: "  << solver.conflictsRestarts << "\n";
  out << b << "nbstopsrestarts: "    << solver.nbstopsrestarts << "\n";
  out << b << "nbstopsrestartssame: "<< solver.nbstopsrestartssame << "\n";
  out << b << "lastblockatrestart: " << solver.lastblockatrestart << "\n";
  out << b << "dec_vars: "           << solver.dec_vars << "\n";
  out << b << "num_clauses: "        << solver.nClauses() << "\n";
  out << b << "num_learnts: "        << solver.nLearnts() << "\n";
  out << b << "clauses_literals: "   << solver.clauses_literals << "\n";
  out << b << "learnts_literals: "   << solver.learnts_literals << "\n";
  out << b << "max_literals: "       << solver.max_literals << "\n";
  out << b << "tot_literals: "       << solver.tot_literals << "\n";
}

void Glucose3xSolver::set_option(std::string&) {
  // TODO
}


std::string Glucose3xSolver::version(int l) {
  if (l&1) return "Glucose 3.X.0";
  return "3.X";
}
