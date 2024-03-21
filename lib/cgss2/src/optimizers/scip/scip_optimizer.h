#ifndef SCIP_OPTIMIZER_H
#define SCIP_OPTIMIZER_H

#include "../../optimizer.h"
#include "../../options.h"

#include <scip/scip.h>
#include <scip/scipdefplugins.h>

#include <vector>


namespace cgss2 {

class SCIPOptimizer : Optimizer {
  SCIP* scip;
  SCIP* lpscip;
  std::vector<SCIP_VAR*> scipvars;
  std::vector<SCIP_VAR*> lpscipvars;

  std::vector<long long> wght;
  std::vector<int> obj_lits;

  long long objective_shift;

  long long convert_expr(const std::vector<long long>& coeffs, const std::vector<int>& lits, std::vector<SCIP_Real>& rcoeffs, std::vector<SCIP_VAR*>& rvars, std::vector<SCIP_VAR*>& lprvars);

public:
  //add constraint:  sum_i coeffs[i]*lits[i] >= rhs
  void add_constr_(std::vector<long long>& coeffs, const std::vector<int>& lits, long long rhs);

  // set objective: minimize sum_i coeffs[i]*lits[i].
  void set_objective_(std::vector<long long>& coeffs, const std::vector<int>& lits);

  // returns 1 if feasible solution found, 2 if an optimal solution is found, 0 if model is infeasible, -1 if nothing of these
  int optimize(std::vector<int>& ret_model, long long bound);
  int optimize_limited(std::vector<int>& ret_model, double time, long long bound);

  // returns 1 if feasible solution found, 0 if model is infeasible
  bool solve_lp(std::vector<double>& model, std::vector<double>& rcs);

  void print_stats(std::string bg, std::ostream& out);

  static std::string version(int l = 0);

  static void add_options(Options& op);

  int init();

  SCIPOptimizer(Options& op);

  ~SCIPOptimizer();
};

}

#endif
