#ifndef OPTIMIZER_H
#define OPTIMIZER_H
#include <vector>
#include <string>
#include <limits>

#include "options.h"
#include "global.h"

namespace cgss2 {

class Optimizer {
public:
  //add constraint:  sum_i coeffs[i]*lits[i] >= cmp
  virtual void add_constr_(std::vector<long long>& coeffs, const std::vector<int>& lits, long long cmp) = 0;
  void add_constr(const std::vector<long long>& coeffs_, const std::vector<int>& lits, long long cmp) {
    std::vector<long long> coeffs(coeffs_);
    add_constr_(coeffs, lits, cmp);
  }
  void add_clause(std::vector<int>& lits, bool isDimacs = 1) {
    std::vector<long long> coeffs(lits.size(), 1);
    if (!isDimacs) for (int& l : lits) l = toDimacs(l);
    add_constr_(coeffs, lits, 1);
    if (!isDimacs) for (int& l : lits) l = fromDimacs(l);
  }

  // set objective: minimize sum_i coeffs[i]*lits[i].
  virtual void set_objective_(std::vector<long long>& coeffs, const std::vector<int>& lits) = 0;
  void set_objective(const std::vector<long long>& coeffs_, const std::vector<int>& lits) {
    std::vector<long long> coeffs(coeffs_);
    set_objective_(coeffs, lits);
  }

  // returns 1 if feasible solution found, 2 if the found solution is quaranteed to be optimal, 0 if model is infeasible, -1 if not solved
  // The search may stop when a solution with cost less or equal to bound is found
  virtual int optimize(std::vector<int>& model, long long bound=std::numeric_limits<long long>::min()) = 0;
  virtual int optimize_limited(std::vector<int>& model, double limit, long long bound=std::numeric_limits<long long>::min()) = 0;

  // returns 1 if feasible solution found, 0 if model is infeasible
  virtual bool solve_lp(std::vector<double>& model, std::vector<double>& rcs) = 0;

  virtual void print_stats(std::string bg, std::ostream& out) = 0;

  virtual ~Optimizer() {}
};

std::vector<std::string> optimizer_available();

std::string optimizer_default();

Optimizer* optimizer_create(std::string name, Options& op);

void optimizer_add_options(Options& op);

std::string optimizer_version(std::string name, int l = 0);


}

#endif
