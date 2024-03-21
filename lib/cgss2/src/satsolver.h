#ifndef SATSOLVER_H
#define SATSOLVER_H

#include <vector>
#include <string>

namespace cgss2{

class SATsolver {

public:
  virtual void add_clause(const std::vector<int>& clause) = 0;

  virtual void add_clause(int lit) = 0;

  virtual void add_clause(int lit1, int lit2) = 0;

  virtual void add_clause(int lit1, int lit2, int lit3) = 0;

  void add_clauses(std::vector<std::vector<int> >::const_iterator b, std::vector<std::vector<int> >::const_iterator e) {
    while (b!=e) add_clause(*b++);
  };

  void add_clauses(const std::vector<std::vector<int> >& clauses) {
    add_clauses(clauses.begin(), clauses.end());
  };


  virtual bool solve(std::vector<int>& assumptions) = 0; // returns true if SAT, false if UNSAT

  virtual bool solve(int lit) = 0; // returns true if SAT, false if UNSAT

  virtual bool solve() = 0; // returns true if SAT, false if UNSAT

  virtual void set_budget(int64_t max_conflicts) = 0;

  virtual void set_budget_relative(double budget) = 0;

  virtual void increase_budget(int64_t max_conflicts) = 0;

  virtual void increase_budget_relative(double budget) = 0;

  virtual int solve_limited(std::vector<int>& assumptions) = 0; // returns 1 if sat, 0 if unsat, -1 if not determined

  virtual int core_size() = 0;

  virtual void get_core(std::vector<int>& retCore) = 0;

  virtual void get_model(std::vector<bool>& retModel) = 0;

  virtual bool propagate(std::vector<int>& assumptions, std::vector<int>& result) = 0; //UP from assumptions, returns true if SAT, false if UNSAT, propagated literals in result

  virtual bool propagate(int lit, std::vector<int>& result) = 0; // UP from lit,

  virtual bool prop_check(std::vector<int>& assumptions) = 0; // UP from assumptions, returns true if SAT, false if UNSAT

  virtual bool prop_check(int lit) = 0;

  virtual void get_learnt_clauses(std::vector<std::vector<int> >& clauses) = 0;

  virtual void set_option(std::string& opt) = 0;

  virtual void stats(const std::string& b, std::ostream& out) = 0;

  virtual ~SATsolver() {};
};

std::vector<std::string> satsolver_available();

std::string satsolver_default();

SATsolver* satsolver_create(std::string& name);

std::string satsolver_version(std::string name, int l = 0);

}

#endif
