#ifndef GUROBI_OPTIMIZER_H
#define GUROBI_OPTIMIZER_H

#include "../../optimizer.h"
#include "../../options.h"
#include <gurobi_c++.h>

#include <vector>

namespace cgss2 {

class GurobiOptimizer : Optimizer {
  GRBEnv* env;
  GRBModel* model;
  std::vector<GRBVar> grbvars;
  GRBModel* lpmodel;
  std::vector<GRBVar> lpgrbvars;

  long long convert_expr(const std::vector<long long>& coeffs, const std::vector<int>& lits, std::vector<double>& rcoeffs, std::vector<GRBVar>& rvars, std::vector<GRBVar>& lprvars);

  long long objective_shift;

public:
  //add constraint:  sum_i coeffs[i]*lits[i] >= rhs
  void add_constr_(std::vector<long long>& coeffs, const std::vector<int>& lits, long long rhs);

  // set objective: minimize sum_i coeffs[i]*lits[i].
  void set_objective_(std::vector<long long>& coeffs, const std::vector<int>& lits);

  // returns 1 if feasible solution found, 2 if an optimal solution is found, 0 if model is infeasible
  int optimize(std::vector<int>& ret_model, long long bound);
  int optimize_limited(std::vector<int>& ret_model, double, long long bound) {
    return optimize(ret_model, bound); // TODO: implement optimize with limit
  }

  // returns 1 if feasible solution found, 0 if model is infeasible
  bool solve_lp(std::vector<double>& model, std::vector<double>& rcs);

  void print_stats(std::string bg, std::ostream& out);

  static std::string version(int l = 0);

  static void add_options(Options& op);

  GurobiOptimizer(Options& op);

  ~GurobiOptimizer();
};

}

#endif
