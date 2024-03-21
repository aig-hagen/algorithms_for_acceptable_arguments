#ifndef GLUCOSE3X_SOLVER_H
#define GLUCOSE3X_SOLVER_H

#include "../../satsolver.h"
#include "core/Solver.h"

#include <vector>

class Glucose3xSolver : cgss2::SATsolver {
  Glucose3xCGSS2::Solver solver;
  int topv;

  void intsToLits(const std::vector<int>& lits, Glucose3xCGSS2::vec<Glucose3xCGSS2::Lit>& rLits) {
    rLits.capacity(lits.size());
    for (unsigned i=0;i<lits.size();++i) {
      while ((lits[i]>>1)>topv) {solver.newVar();++topv;}
      rLits.push_(Glucose3xCGSS2::toLit(lits[i]));
    }
  }

  void litsToInts(Glucose3xCGSS2::vec<Glucose3xCGSS2::Lit>& lits, std::vector<int>& rLits, bool neg = false) {
    rLits.resize(lits.size());
    if (!neg)
      for (int i=0; i<lits.size(); ++i) rLits[i]=Glucose3xCGSS2::toInt(lits[i]);
    else
      for (int i=0; i<lits.size(); ++i) rLits[i]=Glucose3xCGSS2::toInt(~lits[i]);
  }

  void lboolsToBools(Glucose3xCGSS2::vec<Glucose3xCGSS2::lbool>& lbools, std::vector<bool>& bools) {
    bools.resize(lbools.size());
    for (int i=0;i<lbools.size(); ++i) bools[i] = (lbools[i] == g3xl_TrueP ? 1 : 0);
  }

public:
  void add_clause(const std::vector<int>& clause);

  void add_clause(int lit);

  void add_clause(int lit1, int lit2);

  void add_clause(int lit1, int lit2, int lit3) ;

  bool solve(std::vector<int>& assumptions); // returns true if SAT, false if UNSAT

  bool solve(int lit); // return true if SAT, false if UNSAT

  bool solve(); // returns true if SAT, false if UNSAT

  void set_budget(int64_t max_conflicts);

  void set_budget_relative(double budget);

  void increase_budget(int64_t max_conflicts);

  void increase_budget_relative(double budget);

  int solve_limited(std::vector<int>& assumptions); // returns 1 if sat, 0 if unsat, -1 if not determined

  int core_size();

  void get_core(std::vector<int>& retCore);

  void get_model(std::vector<bool>& retModel);

  bool propagate(std::vector<int>& assumptions, std::vector<int>& result); //UP from assumptions, returns true if SAT, false if UNSAT, propagated literals in result

  bool propagate(int lit, std::vector<int>& result); // UP from lit,

  bool prop_check(std::vector<int>& assumptions); // UP from assumptions, returns true if SAT, false if UNSAT

  bool prop_check(int lit);

  void get_learnt_clauses(std::vector<std::vector<int> >& clauses);

  void stats(const std::string& b, std::ostream& out);

  void set_option(std::string& opt);

  static std::string version(int l = 0);

  Glucose3xSolver() : topv(-1) {}
};


#endif
