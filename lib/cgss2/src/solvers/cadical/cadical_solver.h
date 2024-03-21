#ifndef CADICAL_SOLVER_H
#define CADICAL_SOLVER_H

#include "../../satsolver.h"
#include "../../../cadical/src/ccadical.h"

#include <vector>

class CadicalSolver : cgss2::SATsolver{
  CCaDiCaL* solver;

  std::vector<int> prev_assumps;
  int max_var;
public:
  void add_clause(const std::vector<int>& clause);

  void add_clause(int lit);

  void add_clause(int lit1, int lit2);

  void add_clause(int lit1, int lit2, int lit3) ;

  bool solve(std::vector<int>& assumptions); // returns true if SAT, false if UNSAT

  bool solve(int lit); // returns true if SAT, false if UNSAT

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

  CadicalSolver();
};


#endif
