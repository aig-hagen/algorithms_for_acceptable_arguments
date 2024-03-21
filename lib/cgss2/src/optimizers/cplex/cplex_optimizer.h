#ifndef CPLEX_OPTIMIZER_H
#define CPLEX_OPTIMIZER_H

#include "../../optimizer.h"
#include "../../options.h"
#include <ilcplex/ilocplex.h>

#include <vector>

namespace cgss2 {

class CPLEXOptimizer : Optimizer {
  IloEnv env;
  IloModel model;
  IloModel lpmodel;
  IloCplex cplex;
  IloCplex lpcplex;
  std::vector<IloNumVar> cplexvars;
  std::vector<IloNumVar> lpcplexvars;

  long long objective_shift;

  long long convert_expr(std::vector<long long>& coeffs, const std::vector<int>& lits, std::vector<IloNumVar>& rvars, std::vector<IloNumVar>& lprvars);
public:
  // add constraint:  sum_i coeffs[i]*lits[i] >= rhs
  void add_constr_(std::vector<long long>& coeffs, const std::vector<int>& lits, long long rhs);

  // set objective: minimize sum_i coeffs[i]*lits[i].
  void set_objective_(std::vector<long long>& coeffs, const std::vector<int>& lits);

  int optimize(std::vector<int>& ret_model, long long bound);
  int optimize_limited(std::vector<int>& ret_model, double, long long bound) {
    return optimize(ret_model, bound); // TODO: implement optimize with limit
  }

  bool solve_lp(std::vector<double>& model, std::vector<double>& rcs);

  void print_stats(std::string bg, std::ostream& out);

  static std::string version(int l = 0);

  static void add_options(Options& op);


  CPLEXOptimizer(Options& op);

  ~CPLEXOptimizer();
};

}

#endif
